#include "pch.hpp"
#include "BasePin.hpp"

#include "EnumMediaTypes.hpp"

using namespace std;

/* [memo]
 - connection process
  [ Input ]                   [ Output ]
							->  IPin::Connect
								 |
								Check proposed media type
								 |
	IPin::ReceiveConnection	<-	OK
	 |
	Check proposed media
	type
	 |
	Finalize for connection
	 |
	OK                       ->  Finalize for connection

	(remark)
	Input can swap output (however, the most filters doesnÅft support)
*/

HRESULT BasePin::Connect(IPin* receivePin, AM_MEDIA_TYPE const* mediaType)
{
	PrintDebugLog(L"Start");
	CheckPointer(receivePin);

	lock_guard<recursive_mutex> lock(mutex_);
	if (IsConnected()) return VFW_E_ALREADY_CONNECTED;
	if (!IsStopped()) return VFW_E_NOT_STOPPED;

	HRESULT hr = AgreeMediaType(receivePin, mediaType);
	if (FAILED(hr))
	{
		assert(SUCCEEDED(BreakConnection()));
		return hr;
	}

	return S_OK;
}

HRESULT BasePin::AgreeMediaType(IPin* receivePin, AM_MEDIA_TYPE const* mediaType)
{
	PrintDebugLog(L"Start");
	if (mediaType != nullptr)
	{
		auto& mt = *reinterpret_cast<MediaType const*>(mediaType);
		if (!mt.IsPartiallySpecified())
		{
			return TryConnection(receivePin, mediaType);
		}
	}

	::Microsoft::WRL::ComPtr<IEnumMediaTypes> enumMediaTypes;
	HRESULT hr = S_OK;

	hr = receivePin->EnumMediaTypes(&enumMediaTypes);
	if (SUCCEEDED(hr))
	{
		hr = TryMediaTypes(receivePin, mediaType, enumMediaTypes.Get());
		if (SUCCEEDED(hr))
		{
			return hr;
		}

		enumMediaTypes.Reset();
	}

	hr = EnumMediaTypes(&enumMediaTypes);
	if (SUCCEEDED(hr))
	{
		hr = TryMediaTypes(receivePin, mediaType, enumMediaTypes.Get());
	}

	return hr;
}

HRESULT BasePin::TryMediaTypes(IPin* receivePin, AM_MEDIA_TYPE const* mediaType, IEnumMediaTypes* enumMediaTypes)
{
	PrintDebugLog(L"Start");
	HRESULT hr = S_OK;

	auto mt = reinterpret_cast<MediaType const*>(mediaType);

	AM_MEDIA_TYPE* retrivedMediaType = nullptr;
	for (
		HRESULT lhr = enumMediaTypes->Next(1, &retrivedMediaType, nullptr);
		SUCCEEDED(lhr);
		lhr = enumMediaTypes->Next(1, &retrivedMediaType, nullptr))
	{
		if (retrivedMediaType == nullptr)
		{
			return VFW_E_NO_ACCEPTABLE_TYPES;
		}
		if (mt == nullptr || mt->MatchesPartial(*retrivedMediaType))
		{
			hr = TryConnection(receivePin, retrivedMediaType);
		}

		DeleteMediaType(*retrivedMediaType);
		retrivedMediaType = nullptr;

		if (SUCCEEDED(hr))
		{
			return hr;
		}
	}
	return VFW_E_NO_ACCEPTABLE_TYPES;
}

HRESULT BasePin::TryConnection(IPin* receivePin, AM_MEDIA_TYPE const* mediaType)
{
	PrintDebugLog(L"Start");

	HRESULT hr = CheckConnection(receivePin);
	if (FAILED(hr))
	{
		assert(SUCCEEDED(BreakConnection()));
		return hr;
	}

	hr = CheckMediaType(*mediaType);
	if (hr != S_OK)
	{
		assert(SUCCEEDED(BreakConnection()));
		return hr == S_FALSE ? VFW_E_NO_ACCEPTABLE_TYPES : hr;
	}

	hr = SetMediaType(*mediaType);
	if (FAILED(hr))
	{
		assert(SUCCEEDED(BreakConnection()));
		return hr;
	}

	hr = receivePin->ReceiveConnection(this, mediaType);
	if (FAILED(hr))
	{
		assert(SUCCEEDED(BreakConnection()));
		return hr;
	}

	hr = CompleteConnection(receivePin);
	if (FAILED(hr))
	{
		assert(SUCCEEDED(BreakConnection()));
		receivePin->Disconnect();
		return hr;
	}

	pin_ = receivePin;
	return hr;
}

HRESULT BasePin::ReceiveConnection(IPin* receivePin, AM_MEDIA_TYPE const* mediaType)
{
	PrintDebugLog(L"Start");
	CheckPointer(receivePin);
	CheckPointer(mediaType);

	HRESULT hr = S_OK;
	lock_guard<recursive_mutex> lock(mutex_);
	if (IsConnected()) return VFW_E_ALREADY_CONNECTED;
	if (!IsStopped()) return VFW_E_NOT_STOPPED;

	hr = CheckConnection(receivePin);
	if (FAILED(hr))
	{
		assert(SUCCEEDED(BreakConnection()));
		return hr;
	}

	hr = CheckMediaType(*mediaType);
	if (FAILED(hr))
	{
		assert(SUCCEEDED(BreakConnection()));
		return hr;
	}

	hr = SetMediaType(*mediaType);
	if (FAILED(hr))
	{
		assert(SUCCEEDED(BreakConnection()));
		return hr;
	}

	hr = CompleteConnection(receivePin);
	if (FAILED(hr))
	{
		assert(SUCCEEDED(BreakConnection()));
		return hr;
	}

	pin_ = receivePin;
	return hr;
}

HRESULT BasePin::ConnectedTo(IPin** pin)
{
	PrintDebugLog(L"Start");
	CheckPointer(pin);

	pin_.CopyTo(pin);
	return pin_ ? S_OK : VFW_E_NOT_CONNECTED;
}

HRESULT BasePin::Disconnect()
{
	PrintDebugLog(L"Start");
	lock_guard<recursive_mutex> lock(mutex_);
	if (!IsStopped()) return VFW_E_NOT_STOPPED;
	return DisconnectInternal();
}

HRESULT BasePin::DisconnectInternal()
{
	PrintDebugLog(L"Start");
	if (IsConnected())
	{
		HRESULT hr = BreakConnection();
		if (FAILED(hr)) return hr;

		pin_.Reset();
		return S_OK;
	}
	return S_FALSE;
}

HRESULT BasePin::ConnectionMediaType(AM_MEDIA_TYPE* mediaType)
{
	PrintDebugLog(L"Start");
	lock_guard<recursive_mutex> lock(mutex_);
	if (!IsConnected())
	{
		mediaType = CreateMediaType();
		InitializeMediaType(*mediaType);
		return VFW_E_NOT_CONNECTED;
	}

	return mediaType_.CopyTo(*mediaType);
}

HRESULT BasePin::EnumMediaTypes(IEnumMediaTypes** enumMediaTypes)
{
	PrintDebugLog(L"Start");
	AM_MEDIA_TYPE mt = { 0 };
	::std::vector<AM_MEDIA_TYPE*> mediaTypes;
	int i = 0;
	while (GetPreferredMediaType(i++, &mt) == S_OK)
	{
		AM_MEDIA_TYPE* nmt = CreateMediaType();
		InitializeMediaType(*nmt);
		CopyMediaType(mt, *nmt);
		mediaTypes.emplace_back(nmt);
	}
	return EnumMediaTypesFactory::CreateInstance(mediaTypes, enumMediaTypes);
}

HRESULT BasePin::SetMediaType(AM_MEDIA_TYPE const& mediaType)
{
	PrintDebugLog(L"Start");
	return mediaType_.Set(mediaType);
}