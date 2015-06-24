#include "pch.hpp"
#include "TransformOutputPin.hpp"

#include "TransformFilter.hpp"

using namespace std;

#define tparent_ (static_cast<TransformFilter*>(parent_))

TransformOutputPin::TransformOutputPin(TransformFilter* const filter)
	: BaseOutputPin(filter)
{ }

HRESULT TransformOutputPin::Notify(IBaseFilter* sender, Quality quality)
{
	PrintDebugLog(L"Start");
	return tparent_->Notify(sender, quality);
}

HRESULT TransformOutputPin::CheckConnection(IPin* receivePin)
{
	PrintDebugLog(L"Start");
	HRESULT hr = tparent_->CheckOutputPinConnection(receivePin);
	if (FAILED(hr)) return hr;
	return BaseOutputPin::CheckConnection(receivePin);
}

HRESULT TransformOutputPin::CompleteConnection(IPin* receivePin)
{
	PrintDebugLog(L"Start");
	HRESULT hr = tparent_->CompleteOutputPinConnection(receivePin);
	if (FAILED(hr)) return hr;
	return BaseOutputPin::CompleteConnection(receivePin);
}

HRESULT TransformOutputPin::BreakConnection()
{
	PrintDebugLog(L"Start");
	assert(IsStopped());
	tparent_->BreakOutputPinConnection();
	return BaseOutputPin::BreakConnection();
}

HRESULT TransformOutputPin::Receive(IMediaSample* mediaSample)
{
	PrintDebugLog(L"Start");
	return memInputPin_->Receive(mediaSample);
}

HRESULT TransformOutputPin::CheckMediaType(AM_MEDIA_TYPE const& mediaType)
{
	PrintDebugLog(L"Start");
	return tparent_->CheckOutputPinMediaType(&mediaType);
}

HRESULT TransformOutputPin::SetMediaType(AM_MEDIA_TYPE const& mediaType)
{
	PrintDebugLog(L"Start");
	HRESULT hr = BaseOutputPin::SetMediaType(mediaType);
	if (FAILED(hr)) return hr;

	assert(tparent_->IsInputPinConnected());
	return tparent_->SetOutputPinMediaType(&mediaType);
}

HRESULT TransformOutputPin::GetPreferredMediaType(ULONG position, AM_MEDIA_TYPE* mediaType)
{
	PrintDebugLog(L"Start");
	return tparent_->GetOutputPreferredMediaType(position, mediaType);
}

HRESULT TransformOutputPin::DecideBufferSize(_In_ IMemAllocator* memAllocator, _Inout_ ALLOCATOR_PROPERTIES* properties)
{
	PrintDebugLog(L"Start");
	return tparent_->DecideBufferSize(memAllocator, properties);
}