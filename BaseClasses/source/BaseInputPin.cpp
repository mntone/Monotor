#include "pch.hpp"
#include "BaseInputPin.hpp"

using namespace std;

BaseInputPin::BaseInputPin(BaseFilter* const filter)
	: BasePin(filter)
	, flushing_(false)
	, readonly_(false)
{
	name_ = L"In";
}

HRESULT BaseInputPin::QueryInterfaceOverride(IID const& iid, _COM_Outptr_ void** ret)
{
	PrintDebugLog(L"Start");
	if (iid == IID_IMemInputPin)
	{
		AddRefInternal();
		*ret = static_cast<IMemInputPin*>(this);
		return S_OK;
	}
	*ret = nullptr;
	return BasePin::QueryInterfaceOverride(iid, ret);
}

HRESULT BaseInputPin::Connect(IPin* /*receivePin*/, AM_MEDIA_TYPE const* /*mediaType*/) { PrintDebugLog(L"Start"); return E_UNEXPECTED; }

HRESULT BaseInputPin::QueryPinInfo(PIN_INFO* info)
{
	PrintDebugLog(L"Start");
	HRESULT hr = BasePin::QueryPinInfo(info);
	if (FAILED(hr)) return hr;

	info->dir = PIN_DIRECTION::PINDIR_INPUT;
	return S_OK;
}

HRESULT BaseInputPin::QueryDirection(PIN_DIRECTION* pinDirection)
{
	PrintDebugLog(L"Start");
	CheckPointer(pinDirection);
	*pinDirection = PIN_DIRECTION::PINDIR_INPUT;
	return S_OK;
}

HRESULT BaseInputPin::EndOfStream()
{
	PrintDebugLog(L"Start");
	return S_OK;
}

HRESULT BaseInputPin::BeginFlush()
{
	PrintDebugLog(L"Start");
	lock_guard<recursive_mutex> lock(mutex_);
	assert(!flushing_);
	flushing_ = true;
	return S_OK;
}

HRESULT BaseInputPin::EndFlush()
{
	PrintDebugLog(L"Start");
	lock_guard<recursive_mutex> lock(mutex_);
	assert(flushing_);
	flushing_ = false;
	return S_OK;
}

HRESULT BaseInputPin::Notify(IBaseFilter* /*selfFilter*/, Quality /*quality*/)
{
	PrintDebugLog(L"Start");
	return S_OK;
}

HRESULT BaseInputPin::GetAllocator(IMemAllocator** memAllocator)
{
	PrintDebugLog(L"Start");
	CheckPointer(memAllocator);

	HRESULT hr = S_OK;
	lock_guard<recursive_mutex> lock(mutex_);
	if (!memAllocator_)
	{
		hr = CreateMemoryAllocator(*memAllocator_.GetAddressOf());
		if (FAILED(hr)) return hr;
	}

	hr = memAllocator_.CopyTo(memAllocator);
	return hr;
}

HRESULT BaseInputPin::NotifyAllocator(IMemAllocator* memAllocator, BOOL readonly)
{
	PrintDebugLog(L"Start");
	CheckPointer(memAllocator);

	lock_guard<recursive_mutex> lock(mutex_);
	memAllocator_ = memAllocator;

	readonly_ = readonly == TRUE;
	return S_OK;
}

HRESULT BaseInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES* allocatorProperties)
{
	PrintDebugLog(L"Start");
	CheckPointer(allocatorProperties);

	return E_NOTIMPL;
}

HRESULT BaseInputPin::Receive(IMediaSample* mediaSample)
{
	PrintDebugLog(L"Start");
	CheckPointer(mediaSample);

	HRESULT hr = CheckStreaming();
	if (FAILED(hr)) return hr;

	hr = GetProperties(mediaSample, properties_);
	if (FAILED(hr)) return hr;
	if (!(properties_.dwSampleFlags & AM_SAMPLE_TYPECHANGED)) return S_OK;

	hr = CheckMediaType(*properties_.pMediaType);
	if (FAILED(hr))
	{
		error_ = true;
		EndOfStream();
		parent_->NotifyEvent(EC_ERRORABORT, VFW_E_TYPE_NOT_ACCEPTED, 0);
		return VFW_E_INVALIDMEDIATYPE;
	}

	return hr;
}

HRESULT BaseInputPin::GetProperties(IMediaSample* mediaSample, AM_SAMPLE2_PROPERTIES& properties)
{
	PrintDebugLog(L"Start");
	::Microsoft::WRL::ComPtr<IMediaSample2> mediaSample2;
	HRESULT hr = mediaSample->QueryInterface(mediaSample2.GetAddressOf());
	if (SUCCEEDED(hr))
	{
		hr = mediaSample2->GetProperties(sizeof(AM_SAMPLE2_PROPERTIES), reinterpret_cast<BYTE*>(&properties));
		if (FAILED(hr)) return hr;
	}
	else
	{
		properties.cbData = sizeof(AM_SAMPLE2_PROPERTIES);
		properties.dwTypeSpecificFlags = 0;
		properties.dwSampleFlags = AM_STREAM_MEDIA;
		if (mediaSample->IsDiscontinuity() == S_OK) properties.dwSampleFlags |= AM_SAMPLE_DATADISCONTINUITY;
		if (mediaSample->IsPreroll() == S_OK) properties.dwSampleFlags |= AM_SAMPLE_PREROLL;
		if (mediaSample->IsSyncPoint() == S_OK) properties.dwSampleFlags |= AM_SAMPLE_SPLICEPOINT;
		if (SUCCEEDED(mediaSample->GetTime(&properties.tStart, &properties.tStop)))
		{
			properties.dwSampleFlags |= AM_SAMPLE_TIMEVALID | AM_SAMPLE_STOPVALID;
		}
		if (mediaSample->GetMediaType(&properties.pMediaType) == S_OK) properties.dwSampleFlags |= AM_SAMPLE_TYPECHANGED;
		hr = mediaSample->GetPointer(&properties.pbBuffer);
		properties.lActual = mediaSample->GetActualDataLength();
		properties.cbBuffer = mediaSample->GetSize();
	}

	return hr;
}

HRESULT BaseInputPin::ReceiveMultiple(IMediaSample** mediaSamples, long sampleCount, long* samplesProcessedCount)
{
	PrintDebugLog(L"Start");
	CheckPointer(mediaSamples);
	CheckPointer(samplesProcessedCount);

	HRESULT hr = S_OK;
	*samplesProcessedCount = 0;
	while (sampleCount-- > 0)
	{
		hr = Receive(mediaSamples[*samplesProcessedCount]);
		if (hr != S_OK) break;

		++(*samplesProcessedCount);
	}
	return hr;
}

HRESULT BaseInputPin::ReceiveCanBlock()
{
	PrintDebugLog(L"Start");
	::Microsoft::WRL::ComPtr<IEnumPins> enumPins;
	HRESULT hr = parent_->EnumPins(&enumPins);
	if (FAILED(hr)) return hr;

	::Microsoft::WRL::ComPtr<IPin> pin;
	for (
		HRESULT lhr = enumPins->Next(1, &pin, nullptr);
		FAILED(lhr);
		lhr = enumPins->Next(1, &pin, nullptr))
	{
		if (pin == nullptr) return S_FALSE;

		PIN_DIRECTION direction;
		hr = pin->QueryDirection(&direction);
		if (FAILED(hr)) return hr;

		if (direction == PIN_DIRECTION::PINDIR_OUTPUT)
		{
			::Microsoft::WRL::ComPtr<IPin> connectedPin;
			hr = pin->ConnectedTo(&connectedPin);
			if (SUCCEEDED(hr))
			{
				assert(connectedPin);

				::Microsoft::WRL::ComPtr<IMemInputPin> memInputPin;
				hr = connectedPin.As(&memInputPin);
				if (SUCCEEDED(hr))
				{
					hr = memInputPin->ReceiveCanBlock();
					if (FAILED(hr)) return S_FALSE;
				}
			}
		}

		pin.Reset();
	}
	return S_OK;
}

HRESULT BaseInputPin::CheckStreaming()
{
	PrintDebugLog(L"Start");
	assert(IsConnected());
	if (IsStopped()) return VFW_E_WRONG_STATE;
	if (flushing_) return S_FALSE;
	if (error_) return VFW_E_RUNTIME_ERROR;
	return S_OK;
}

HRESULT BaseInputPin::Inactive()
{
	PrintDebugLog(L"Start");
	error_ = false;
	if (memAllocator_ == nullptr) return VFW_E_NO_ALLOCATOR;
	flushing_ = false;
	return memAllocator_->Decommit();
}

HRESULT BaseInputPin::PassNotify(Quality& quality)
{
	PrintDebugLog(L"Start");
	HRESULT hr = S_OK;

	if (!qualityControl_)
	{
		::Microsoft::WRL::ComPtr<IQualityControl> qualityControl;

		hr = VFW_E_NOT_FOUND;
		if (pin_)
		{
			hr = pin_.CopyTo(qualityControl.GetAddressOf());
			if (FAILED(hr)) return VFW_E_NOT_FOUND;

			hr = VFW_E_NOT_FOUND;
			if (qualityControl)
			{
				hr = qualityControl->Notify(parent_, quality);
			}
		}
	}
	else
	{
		hr = qualityControl_->Notify(parent_, quality);
	}
	return hr;
}

HRESULT BaseInputPin::CompleteConnection(IPin* /*receivePin*/)
{
	PrintDebugLog(L"Start");
	return S_OK;
}

HRESULT BaseInputPin::BreakConnection()
{
	PrintDebugLog(L"Start");
	HRESULT hr = S_OK;

	properties_ = { 0 };

	if (memAllocator_)
	{
		hr = memAllocator_->Decommit();
		if (SUCCEEDED(hr)) memAllocator_.Reset();
	}

	return hr;
}