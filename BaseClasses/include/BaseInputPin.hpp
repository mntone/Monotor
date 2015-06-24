#pragma once
#include "BasePin.hpp"

class __declspec(novtable) BaseInputPin
	: public BasePin
	, public IMemInputPin
{
public:
	// IPin
	virtual HRESULT _stdcall Connect(_In_ IPin* receivePin, _In_opt_ AM_MEDIA_TYPE const* mediaType) override;
	virtual HRESULT _stdcall QueryPinInfo(_Out_ PIN_INFO* info) override final;
	virtual HRESULT _stdcall QueryDirection(_Out_ PIN_DIRECTION* pinDirection) override final;
	virtual HRESULT _stdcall EndOfStream() override;
	virtual HRESULT _stdcall BeginFlush() override;
	virtual HRESULT _stdcall EndFlush() override;

	// IQualityControl
	virtual HRESULT _stdcall Notify(_In_ IBaseFilter* selfFilter, _In_ Quality quality) override;

	// IMemInputPin
	virtual HRESULT _stdcall GetAllocator(_Out_ IMemAllocator** memAllocator) override;
	virtual HRESULT _stdcall NotifyAllocator(_In_ IMemAllocator* memAllocator, _In_ BOOL readonly) override;
	virtual HRESULT _stdcall GetAllocatorRequirements(_Out_ ALLOCATOR_PROPERTIES* allocatorProperties) override;
	virtual HRESULT _stdcall Receive(_In_ IMediaSample* mediaSample) override;
	virtual HRESULT _stdcall ReceiveMultiple(_In_reads_(sampleCount) IMediaSample** mediaSamples, _In_ long sampleCount, _Out_ long* samplesProcessedCount) override final;
	virtual HRESULT _stdcall ReceiveCanBlock() override final;

	virtual HRESULT _stdcall CheckStreaming();

	HRESULT _stdcall Inactive();

	HRESULT _stdcall PassNotify(_In_ Quality& quality);

	bool IsReadonly() const { return readonly_; }
	bool IsFlushing() const { return flushing_; }
	AM_SAMPLE2_PROPERTIES const& GetProperties() const { assert(properties_.cbData != 0); return properties_; }

	DECLARE_IUNKNOWN;

protected:
	BaseInputPin(_In_ BaseFilter* const filter);

	virtual HRESULT _stdcall QueryInterfaceOverride(IID const& iid, _COM_Outptr_ void** ret) override;

	virtual HRESULT _stdcall CompleteConnection(_In_ IPin* receivePin);
	virtual HRESULT _stdcall BreakConnection();

private:
	BaseInputPin() = delete;
	BaseInputPin(BaseInputPin const&) = delete;
	BaseInputPin(BaseInputPin&&) = delete;

	BaseInputPin& operator=(BaseInputPin const&) = delete;
	BaseInputPin& operator=(BaseInputPin&&) = delete;

	HRESULT _stdcall GetProperties(_In_ IMediaSample* mediaSample, _Out_ AM_SAMPLE2_PROPERTIES& properties);

protected:
	::Microsoft::WRL::ComPtr<IMemAllocator> memAllocator_;
	bool flushing_, readonly_;
	AM_SAMPLE2_PROPERTIES properties_;
};