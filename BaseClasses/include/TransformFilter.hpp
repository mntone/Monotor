#pragma once
#include "BaseFilter.hpp"
#include "TransformInputPin.hpp"
#include "TransformOutputPin.hpp"
#include "../DirectShowEx_h.h"

class __declspec(novtable) TransformFilter
	: public BaseFilter
	, public ITransformInputPinProxy
	, public ITransformOutputPinProxy
{
public:
	// IMediaFilter
	virtual HRESULT _stdcall Stop() override;
	virtual HRESULT _stdcall Pause() override;
	virtual HRESULT _stdcall Run(_In_ REFERENCE_TIME time) override;
	
	// IBaseFilter
	virtual HRESULT _stdcall EnumPins(_Out_ IEnumPins** enumPins) override;
	virtual HRESULT _stdcall FindPin(_In_ LPCWSTR id, _Out_ IPin** pin) override;

	// IInputPinProxy
	virtual HRESULT _stdcall EndOfStream() override;
	virtual HRESULT _stdcall BeginFlush() override;
	virtual HRESULT _stdcall EndFlush() override;
	virtual HRESULT _stdcall NewSegment(_In_ REFERENCE_TIME startTime, _In_ REFERENCE_TIME stopTime, _In_ double rate) override;
	
	virtual HRESULT _stdcall Receive(_In_ IMediaSample* mediaSample) override;

	virtual HRESULT _stdcall CheckInputPinConnection(_In_ IPin* receivePin) override;
	virtual HRESULT _stdcall CompleteInputPinConnection(_In_ IPin* receivePin) override;
	virtual HRESULT _stdcall BreakInputPinConnection() override;
	virtual HRESULT _stdcall CheckInputPinMediaType(_In_ AM_MEDIA_TYPE const* mediaType) override final;
	virtual HRESULT _stdcall SetInputPinMediaType(_In_ AM_MEDIA_TYPE const* mediaType) override;

	// IOutputPinProxy
	virtual HRESULT _stdcall CheckOutputPinConnection(_In_ IPin* receivePin) override;
	virtual HRESULT _stdcall CompleteOutputPinConnection(_In_ IPin* receivePin) override;
	virtual HRESULT _stdcall BreakOutputPinConnection() override;
	virtual HRESULT _stdcall CheckOutputPinMediaType(_In_ AM_MEDIA_TYPE const* mediaType) override final;
	virtual HRESULT _stdcall SetOutputPinMediaType(_In_ AM_MEDIA_TYPE const* mediaType) override;

	virtual HRESULT _stdcall Notify(_In_ IBaseFilter* sender, _In_ Quality quality);
	virtual HRESULT _stdcall DecideBufferSize(_In_ IMemAllocator* memAllocator, _Inout_ ALLOCATOR_PROPERTIES* properties) = 0;

	virtual HRESULT _stdcall StartStreaming();
	virtual HRESULT _stdcall StopStreaming();

	::std::recursive_mutex& Mutex() { return mutex_; }

	bool IsInputPinConnected() const { return inputPin_.IsConnected(); }
	bool IsOutputPinConnected() const { return outputPin_.IsConnected(); }

	DECLARE_IUNKNOWN;

protected:
	TransformFilter(_In_ CLSID const& clsID);

	virtual HRESULT _stdcall QueryInterfaceOverride(IID const& iid, _COM_Outptr_ void** ret) override;

	virtual HRESULT _stdcall CheckMediaType(_In_ AM_MEDIA_TYPE const* inputPinMediaType, _In_opt_ AM_MEDIA_TYPE const* outputPinMediaType) = 0;

	virtual HRESULT _stdcall CreateBuffer(_In_ IMediaSample* mediaSample, _Outptr_opt_ IMediaSample*& newMediaSample);
	virtual HRESULT _stdcall Transform(_In_ IMediaSample* mediaSample, _Inout_ IMediaSample* newMediaSample);
	virtual HRESULT _stdcall TransformOverride(_In_ BYTE* src, _In_ int32_t srcSize, _In_ BYTE* dst, _Inout_ int32_t* dstSize) = 0;

private:
	TransformFilter() = delete;
	TransformFilter(TransformFilter const&) = delete;
	TransformFilter(TransformFilter&&) = delete;

	TransformFilter& operator=(TransformFilter const&) = delete;
	TransformFilter& operator=(TransformFilter&&) = delete;

protected:
	bool eosDelivered_, skipped_, qualityChanged_;
	TransformInputPin inputPin_;
	TransformOutputPin outputPin_;
};