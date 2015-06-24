#include "pch.hpp"
#include "TransformFilter.hpp"

#include "EnumPins.hpp"

#define TRANSFORM_ID 1

using namespace std;

TransformFilter::TransformFilter(CLSID const& clsID)
	: BaseFilter(clsID)
	, eosDelivered_(false)
	, skipped_(false)
	, qualityChanged_(false)
	, inputPin_(this)
	, outputPin_(this)
{ }

HRESULT TransformFilter::QueryInterfaceOverride(IID const& iid, void** ret)
{
	PrintDebugLog(L"Start");
	CheckPointer(ret);
	if (iid == IID_ITransformInputPinProxy)
	{
		AddRefInternal();
		*ret = static_cast<ITransformInputPinProxy*>(this);
		return S_OK;
	}
	if (iid == IID_ITransformOutputPinProxy)
	{
		AddRefInternal();
		*ret = static_cast<ITransformOutputPinProxy*>(this);
		return S_OK;
	}
	return BaseFilter::QueryInterfaceOverride(iid, ret);
}

HRESULT TransformFilter::Stop()
{
	PrintDebugLog(L"Start");
	HRESULT hr = S_OK;
	lock_guard<recursive_mutex> lock(mutex_);
	if (state_ != filter_state::stopped)
	{
		if (!IsInputPinConnected())
		{
			if (!eosDelivered_)
			{
				outputPin_.DeliverEndOfStream();
				eosDelivered_ = true;
			}
		}

		inputPin_.Inactive();
		outputPin_.Inactive();

		hr = StopStreaming();
		if (SUCCEEDED(hr))
		{
			skipped_ = false;
			qualityChanged_ = false;
			state_ = filter_state::stopped;
		}
	}
	return hr;
}

HRESULT TransformFilter::Pause()
{
	PrintDebugLog(L"Start");
	HRESULT hr = S_OK;
	lock_guard<recursive_mutex> lock(mutex_);
	if (state_ != filter_state::paused)
	{
		if (!IsInputPinConnected())
		{
			if (IsOutputPinConnected() && !eosDelivered_)
			{
				outputPin_.DeliverEndOfStream();
				eosDelivered_ = true;
			}
			state_ = filter_state::paused;
		}
		else if (!IsOutputPinConnected())
		{
			state_ = filter_state::paused;
		}
		else if (state_ == filter_state::stopped)
		{
			hr = StartStreaming();
			if (SUCCEEDED(hr))
			{
				hr = outputPin_.Active();
				if (SUCCEEDED(hr))
				{
					state_ = filter_state::paused;
				}
			}
		}
	}

	skipped_ = false;
	qualityChanged_ = false;
	return hr;
}

HRESULT TransformFilter::Run(REFERENCE_TIME time)
{
	PrintDebugLog(L"Start");
	HRESULT hr = S_OK;
	lock_guard<recursive_mutex> lock(mutex_);
	if (state_ == filter_state::stopped) hr = Pause();
	if (FAILED(hr)) return hr;
	if (state_ != filter_state::running)
	{
		if (IsInputPinConnected())
		{
			inputPin_.Run(time);
		}
		if (IsOutputPinConnected())
		{
			outputPin_.Run(time);
		}
	}
	state_ = filter_state::running;
	return hr;
}

HRESULT TransformFilter::StartStreaming()
{
	PrintDebugLog(L"Start");
	return S_OK;
}

HRESULT TransformFilter::StopStreaming()
{
	PrintDebugLog(L"Start");
	return S_OK;
}

HRESULT TransformFilter::EnumPins(IEnumPins** enumPins)
{
	PrintDebugLog(L"Start");
	return EnumPinsFactory::CreateInstance(&inputPin_, &outputPin_, enumPins);
}

HRESULT TransformFilter::FindPin(LPCWSTR id, IPin** pin)
{
	PrintDebugLog(L"Start");
	CheckPointer(pin);

	if (id == inputPin_.Name())
	{
		*pin = &inputPin_;
		InterlockedIncrement(&referenceCount_);
		return S_OK;
	}
	if (id == outputPin_.Name())
	{
		*pin = &outputPin_;
		InterlockedIncrement(&referenceCount_);
		return S_OK;
	}
	return VFW_E_NOT_FOUND;
}

HRESULT TransformFilter::EndOfStream()
{
	PrintDebugLog(L"Start");
	/* MUST NOT LOCK!! (called from TransformInputPin) */
	HRESULT hr = outputPin_.DeliverEndOfStream();
	eosDelivered_ = true;
	return hr;
}

HRESULT TransformFilter::BeginFlush()
{
	PrintDebugLog(L"Start");
	/* MUST NOT LOCK!! (called from TransformInputPin) */
	return outputPin_.DeliverBeginFlush();
}

HRESULT TransformFilter::EndFlush()
{
	PrintDebugLog(L"Start");
	/* MUST NOT LOCK!! (called from TransformInputPin) */
	return outputPin_.DeliverEndFlush();
}

HRESULT TransformFilter::NewSegment(REFERENCE_TIME startTime, REFERENCE_TIME stopTime, double rate)
{
	PrintDebugLog(L"Start");
	return outputPin_.DeliverNewSegment(startTime, stopTime, rate);
}

HRESULT TransformFilter::Receive(IMediaSample* mediaSample)
{
	PrintDebugLog(L"Start");
	assert(mediaSample);

	AM_SAMPLE2_PROPERTIES const& properties = inputPin_.GetProperties();
	if (properties.dwStreamId != AM_STREAM_MEDIA)
	{
		return outputPin_.Receive(mediaSample);
	}

	::Microsoft::WRL::ComPtr<IMediaSample> newMediaSample;
	HRESULT hr = CreateBuffer(mediaSample, *newMediaSample.GetAddressOf());
	if (FAILED(hr)) return hr;

	hr = Transform(mediaSample, newMediaSample.Get());
	if (SUCCEEDED(hr))
	{
		if (hr != S_FALSE)
		{
			hr = outputPin_.Receive(newMediaSample.Get());
			skipped_ = false;
		}
		else
		{
			skipped_ = true;
			if (!qualityChanged_)
			{
				NotifyEvent(EC_QUALITY_CHANGE, 0, 0);
				qualityChanged_ = true;
			}
			return S_OK;
		}
	}

	return hr;
}

HRESULT TransformFilter::CreateBuffer(IMediaSample* mediaSample, IMediaSample*& newMediaSample)
{
	PrintDebugLog(L"Start");

	AM_SAMPLE2_PROPERTIES& properties = const_cast<AM_SAMPLE2_PROPERTIES&>(inputPin_.GetProperties());
	DWORD flags = skipped_ ? AM_GBF_PREVFRAMESKIPPED : 0;
	if (!(properties.dwSampleFlags & AM_SAMPLE_SPLICEPOINT)) flags |= AM_GBF_NOTASYNCPOINT;

	::Microsoft::WRL::ComPtr<IMediaSample> nms;
	HRESULT hr = outputPin_.CreateBuffer(
		(properties.dwSampleFlags & AM_SAMPLE_TIMEVALID) ? &properties.tStart : nullptr,
		(properties.dwSampleFlags & AM_SAMPLE_STOPVALID) ? &properties.tStop : nullptr,
		flags,
		*nms.GetAddressOf());
	if (FAILED(hr)) return hr;

	hr = nms.CopyTo(&newMediaSample);
	if (FAILED(hr)) return hr;

	::Microsoft::WRL::ComPtr<IMediaSample2> nms2;
	hr = nms.As(&nms2);
	if (SUCCEEDED(hr))
	{
		AM_SAMPLE2_PROPERTIES newProperties = { 0 };
		hr = nms2->GetProperties(
			FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, tStart),
			reinterpret_cast<BYTE*>(&newProperties));
		assert(SUCCEEDED(hr));
		newProperties.dwTypeSpecificFlags = properties.dwTypeSpecificFlags;
		newProperties.dwSampleFlags
			= (newProperties.dwSampleFlags & AM_SAMPLE_TYPECHANGED)
			| (properties.dwSampleFlags & ~AM_SAMPLE_TYPECHANGED);
		newProperties.tStart = properties.tStart;
		newProperties.tStart = properties.tStop;
		newProperties.cbData = FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, dwStreamId);

		hr = nms2->SetProperties(
			FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, dwStreamId),
			reinterpret_cast<BYTE*>(&newProperties));

		if (properties.dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY)
		{
			skipped_ = false;
		}
	}
	else
	{
		if (properties.dwSampleFlags & AM_SAMPLE_TIMEVALID)
		{
			nms->SetTime(&properties.tStart, &properties.tStop);
		}
		if (properties.dwSampleFlags & AM_SAMPLE_SPLICEPOINT)
		{
			nms->SetSyncPoint(TRUE);
		}
		if (properties.dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY)
		{
			nms->SetDiscontinuity(TRUE);
			skipped_ = false;
		}

		LONGLONG mediaStart, mediaEnd;
		hr = mediaSample->GetMediaTime(&mediaStart, &mediaEnd);
		if (SUCCEEDED(hr))
		{
			nms->SetMediaTime(&mediaStart, &mediaEnd);
		}
	}

	return S_OK;
}

HRESULT TransformFilter::CheckInputPinConnection(IPin* /*receivePin*/)
{
	PrintDebugLog(L"Start");
	return S_OK;
}

HRESULT TransformFilter::CompleteInputPinConnection(IPin* /*receivePin*/)
{
	PrintDebugLog(L"Start");
	return S_OK;
}

HRESULT TransformFilter::BreakInputPinConnection()
{
	PrintDebugLog(L"Start");
	return S_OK;
}

HRESULT TransformFilter::CheckInputPinMediaType(AM_MEDIA_TYPE const* mediaType)
{
	PrintDebugLog(L"Start");
	return CheckMediaType(mediaType, nullptr);
}

HRESULT TransformFilter::SetInputPinMediaType(AM_MEDIA_TYPE const* /*mediaType*/)
{
	PrintDebugLog(L"Start");
	return S_OK;
}

HRESULT TransformFilter::CheckOutputPinConnection(IPin* /*receivePin*/)
{
	PrintDebugLog(L"Start");
	return S_OK;
}

HRESULT TransformFilter::CompleteOutputPinConnection(IPin* /*receivePin*/)
{
	PrintDebugLog(L"Start");
	return S_OK;
}

HRESULT TransformFilter::BreakOutputPinConnection()
{
	PrintDebugLog(L"Start");
	return S_OK;
}

HRESULT TransformFilter::CheckOutputPinMediaType(AM_MEDIA_TYPE const* mediaType)
{
	PrintDebugLog(L"Start");
	MediaType inputPinMediaType;
	HRESULT hr = inputPin_.ConnectionMediaType(inputPinMediaType.mt_);
	assert(SUCCEEDED(hr));
	return CheckMediaType(inputPinMediaType.mt_, mediaType);
}

HRESULT TransformFilter::SetOutputPinMediaType(AM_MEDIA_TYPE const* /*mediaType*/)
{
	PrintDebugLog(L"Start");
	return S_OK;
}

HRESULT TransformFilter::Notify(IBaseFilter* /*sender*/, Quality quality)
{
	PrintDebugLog(L"Start");
	return inputPin_.PassNotify(quality);
}

HRESULT TransformFilter::Transform(_In_ IMediaSample* mediaSample, _Inout_ IMediaSample* newMediaSample)
{
	PrintDebugLog(L"Start");
	HRESULT hr = S_OK;

	int32_t srcSize = mediaSample->GetActualDataLength();
	int32_t dstSize = newMediaSample->GetSize();
	assert(srcSize <= dstSize);

	BYTE* src;
	hr = mediaSample->GetPointer(&src);
	if (FAILED(hr)) return hr;

	BYTE* dst;
	hr = newMediaSample->GetPointer(&dst);
	if (FAILED(hr)) return hr;

	hr = TransformOverride(src, srcSize, dst, &dstSize);
	if (FAILED(hr)) return hr;

	hr = newMediaSample->SetActualDataLength(dstSize);
	if (FAILED(hr)) return hr;

	REFERENCE_TIME startTime, endTime;
	hr = mediaSample->GetTime(&startTime, &endTime);
	if (SUCCEEDED(hr))
	{
		newMediaSample->SetTime(&startTime, &endTime);
	}

	LONGLONG startMediaTime, endMediaTime;
	hr = mediaSample->GetMediaTime(&startMediaTime, &endMediaTime);
	if (SUCCEEDED(hr))
	{
		newMediaSample->SetMediaTime(&startMediaTime, &endMediaTime);
	}

	return hr;
}