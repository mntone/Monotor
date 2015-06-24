#include "pch.hpp"
#include "TransformInputPin.hpp"

#include "TransformFilter.hpp"

using namespace std;

#define tparent_ (static_cast<TransformFilter*>(parent_))

TransformInputPin::TransformInputPin(TransformFilter* const filter)
	: BaseInputPin(filter)
{ }

HRESULT TransformInputPin::EndOfStream()
{
	PrintDebugLog(L"Start");
	lock_guard<recursive_mutex> lock(tparent_->Mutex());
	HRESULT hr = CheckStreaming();
	if (FAILED(hr)) return hr;

	hr = tparent_->EndOfStream();
	return hr;
}

HRESULT TransformInputPin::BeginFlush()
{
	PrintDebugLog(L"Start");
	lock_guard<recursive_mutex> lock(tparent_->Mutex());
	if (!IsConnected() || !tparent_->IsOutputPinConnected())
	{
		return VFW_E_NOT_CONNECTED;
	}

	HRESULT hr = BaseInputPin::BeginFlush();
	if (FAILED(hr)) return hr;

	hr = tparent_->BeginFlush();
	return hr;
}

HRESULT TransformInputPin::EndFlush()
{
	PrintDebugLog(L"Start");
	lock_guard<recursive_mutex> lock(tparent_->Mutex());
	if (!IsConnected() || !tparent_->IsOutputPinConnected())
	{
		return VFW_E_NOT_CONNECTED;
	}

	HRESULT hr = BaseInputPin::EndFlush();
	if (FAILED(hr)) return hr;

	hr = tparent_->EndFlush();
	return hr;
}

HRESULT TransformInputPin::NewSegment(REFERENCE_TIME startTime, REFERENCE_TIME stopTime, double rate)
{
	PrintDebugLog(L"Start");
	HRESULT hr = BaseInputPin::NewSegment(startTime, stopTime, rate);
	if (FAILED(hr)) return hr;
	hr = tparent_->NewSegment(startTime, stopTime, rate);
	return hr;
}

HRESULT TransformInputPin::Receive(IMediaSample* mediaSample)
{
	PrintDebugLog(L"Start");
	assert(mediaSample);

	lock_guard<recursive_mutex> lock(tparent_->Mutex());

	HRESULT hr = BaseInputPin::Receive(mediaSample);
	if (FAILED(hr)) return hr;

	hr = tparent_->Receive(mediaSample);
	return hr;
}

HRESULT TransformInputPin::CheckStreaming()
{
	PrintDebugLog(L"Start");
	if (!tparent_->IsOutputPinConnected()) return VFW_E_NOT_CONNECTED;
	return BaseInputPin::CheckStreaming();
}

HRESULT TransformInputPin::CheckConnection(IPin* receivePin)
{
	PrintDebugLog(L"Start");
	return tparent_->CheckInputPinConnection(receivePin);
}

HRESULT TransformInputPin::CompleteConnection(IPin* receivePin)
{
	PrintDebugLog(L"Start");
	HRESULT hr = tparent_->CompleteInputPinConnection(receivePin);
	if (FAILED(hr)) return hr;
	return BaseInputPin::CompleteConnection(receivePin);
}

HRESULT TransformInputPin::BreakConnection()
{
	PrintDebugLog(L"Start");
	assert(IsStopped());
	tparent_->BreakInputPinConnection();
	return BaseInputPin::BreakConnection();
}

HRESULT TransformInputPin::CheckMediaType(AM_MEDIA_TYPE const& mediaType)
{
	PrintDebugLog(L"Start");
	return tparent_->CheckInputPinMediaType(&mediaType);
}

HRESULT TransformInputPin::SetMediaType(AM_MEDIA_TYPE const& mediaType)
{
	PrintDebugLog(L"Start");
	HRESULT hr = BaseInputPin::SetMediaType(mediaType);
	if (FAILED(hr)) return hr;

	assert(tparent_->IsOutputPinConnected() && SUCCEEDED(tparent_->CheckOutputPinMediaType(&mediaType)) || !tparent_->IsOutputPinConnected());
	return tparent_->SetInputPinMediaType(&mediaType);
}

HRESULT TransformInputPin::GetPreferredMediaType(ULONG position, AM_MEDIA_TYPE* mediaType)
{
	PrintDebugLog(L"Start");
	return tparent_->GetInputPreferredMediaType(position, mediaType);
}