#pragma once
#include "BaseOutputPin.hpp"

class TransformFilter;
class TransformOutputPin
	: public BaseOutputPin
{
public:
	TransformOutputPin(_In_ TransformFilter* const filter);

	// IQualityControl
	virtual HRESULT _stdcall Notify(_In_ IBaseFilter* sender, _In_ Quality quality) override final;

	virtual HRESULT _stdcall CheckConnection(_In_ IPin* receivePin) override;
	virtual HRESULT _stdcall CompleteConnection(_In_ IPin* receivePin) override;
	virtual HRESULT _stdcall BreakConnection() override;

	HRESULT _stdcall Receive(_In_ IMediaSample* mediaSample);

protected:
	virtual HRESULT _stdcall CheckMediaType(_In_ AM_MEDIA_TYPE const& mediaType) override;
	virtual HRESULT _stdcall SetMediaType(_In_ AM_MEDIA_TYPE const& mediaType) override;
	virtual HRESULT _stdcall GetPreferredMediaType(_In_ ULONG position, _Inout_ AM_MEDIA_TYPE* mediaType) override;

	virtual HRESULT _stdcall DecideBufferSize(_In_ IMemAllocator* memAllocator, _Inout_ ALLOCATOR_PROPERTIES* properties) override;

private:
	TransformOutputPin() = delete;
	TransformOutputPin(TransformOutputPin const&) = delete;
	TransformOutputPin(TransformOutputPin&&) = delete;

	TransformOutputPin& operator=(TransformOutputPin const&) = delete;
	TransformOutputPin& operator=(TransformOutputPin&&) = delete;
};