#pragma once
#include "TransformFilter.hpp"
#include "../AudioMixDownFilter_h.h"
#include "DllSetup.hpp"

class AudioMixDownFilter final
	: public TransformFilter
{
public:
	virtual HRESULT _stdcall GetInputPreferredMediaType(_In_ ULONG position, _Inout_ AM_MEDIA_TYPE* mediaType) override final;
	virtual HRESULT _stdcall SetInputPinMediaType(_In_ AM_MEDIA_TYPE const* mediaType) override;
	virtual HRESULT _stdcall CompleteOutputPinConnection(_In_ IPin* receivePin) override;
	virtual HRESULT _stdcall GetOutputPreferredMediaType(_In_ ULONG position, _Inout_ AM_MEDIA_TYPE* mediaType) override final;
	virtual HRESULT _stdcall SetOutputPinMediaType(_In_ AM_MEDIA_TYPE const* mediaType) override;

	virtual HRESULT _stdcall DecideBufferSize(_In_ IMemAllocator* memAllocator, _Inout_ ALLOCATOR_PROPERTIES* properties) override final;

	static HRESULT _stdcall CreateInstance(_In_ ClassUnloadContext ctx, _Outptr_opt_ IUnknown** ret);

protected:
	virtual HRESULT _stdcall CheckMediaType(_In_ AM_MEDIA_TYPE const* inputPinMediaType, _In_ AM_MEDIA_TYPE const* outputPinMediaType) override final;

	virtual HRESULT _stdcall TransformOverride(_In_ BYTE* src, _In_ int32_t srcSize, _In_ BYTE* dst, _Inout_ int32_t* dstSize) override final;

private:
	AudioMixDownFilter(ClassUnloadContext ctx);

private:
	ClassUnloadContext ctx_;
	WORD inputBit_, outputBit_, outputChannel_;
	::std::function<void(void const*&, void*&)> converter_;
};