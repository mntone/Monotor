#pragma once
#include "BasePin.hpp"

class __declspec(novtable) BaseOutputPin
	: public BasePin
{
public:
	// IPin
	virtual HRESULT _stdcall ReceiveConnection(_In_ IPin* pin, _In_ AM_MEDIA_TYPE const* mediaType) override;
	virtual HRESULT _stdcall QueryPinInfo(_Out_ PIN_INFO* info) override final;
	virtual HRESULT _stdcall QueryDirection(_Out_ PIN_DIRECTION* pinDirection) override final;
	virtual HRESULT _stdcall EndOfStream() override final;
	virtual HRESULT _stdcall BeginFlush() override final;
	virtual HRESULT _stdcall EndFlush() override final;

	// IQualityControl
	virtual HRESULT _stdcall Notify(_In_ IBaseFilter* selfFilter, _In_ Quality quality) override;

	HRESULT _stdcall Active();
	HRESULT _stdcall Inactive();

	HRESULT _stdcall DeliverEndOfStream();
	HRESULT _stdcall DeliverBeginFlush();
	HRESULT _stdcall DeliverEndFlush();
	HRESULT _stdcall DeliverNewSegment(_In_ REFERENCE_TIME startTime, _In_ REFERENCE_TIME stopTime, _In_ double rate);

	HRESULT _stdcall CreateBuffer(_In_opt_ REFERENCE_TIME* startTime, _In_opt_ REFERENCE_TIME* endTime, DWORD flags, _Outptr_opt_ IMediaSample*& mediaSample);

protected:
	BaseOutputPin(_In_ BaseFilter* const filter);

	virtual HRESULT _stdcall CheckConnection(_In_ IPin* receivePin);
	virtual HRESULT _stdcall CompleteConnection(_In_ IPin* receivePin);
	virtual HRESULT _stdcall BreakConnection();

	virtual HRESULT _stdcall DecideAllocator(_Outptr_opt_ IMemAllocator*& memAllocator);
	virtual HRESULT _stdcall DecideBufferSize(_In_ IMemAllocator* memAllocator, _Inout_ ALLOCATOR_PROPERTIES* properties) = 0;

#if _DEBUG
	HRESULT DeliverBuffer(_In_ IMediaSample* mediaSample);
#endif

private:
	BaseOutputPin() = delete;
	BaseOutputPin(BaseOutputPin const&) = delete;
	BaseOutputPin(BaseOutputPin&&) = delete;

	BaseOutputPin& operator=(BaseOutputPin const&) = delete;
	BaseOutputPin& operator=(BaseOutputPin&&) = delete;

protected:
	::Microsoft::WRL::ComPtr<IMemInputPin> memInputPin_;
	::Microsoft::WRL::ComPtr<IMemAllocator> memAllocator_;
};