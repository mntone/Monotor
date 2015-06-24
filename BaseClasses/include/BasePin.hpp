#pragma once
#include "UnknownObject.hpp"
#include "BaseFilter.hpp"
#include "MediaType.hpp"

class __declspec(novtable) BasePin
	: public DelegateUnknownObject
	, public IPin
	, public IQualityControl
{
public:
	// IPin
	virtual HRESULT _stdcall Connect(_In_ IPin* receivePin, _In_opt_ AM_MEDIA_TYPE const* mediaType) override;
	virtual HRESULT _stdcall ReceiveConnection(_In_ IPin* receivePin, _In_ AM_MEDIA_TYPE const* mediaType) override;
	virtual HRESULT _stdcall Disconnect() override;
	virtual HRESULT _stdcall ConnectedTo(_Out_ IPin** pin) override;
	virtual HRESULT _stdcall ConnectionMediaType(_Out_ AM_MEDIA_TYPE* mediaType) override;
	virtual HRESULT _stdcall QueryPinInfo(_Out_ PIN_INFO* info) override;
	virtual HRESULT _stdcall QueryId(_Out_ LPWSTR* id) override final;
	virtual HRESULT _stdcall QueryAccept(_In_ AM_MEDIA_TYPE const* mediaType) override final;
	virtual HRESULT _stdcall EnumMediaTypes(_Out_ IEnumMediaTypes** enumMediaTypes) override final;
	virtual HRESULT _stdcall QueryInternalConnections(_Out_writes_to_opt_(*pinCount, *pinCount) IPin** pin, _Inout_ ULONG* pinCount) override;
	virtual HRESULT _stdcall NewSegment(_In_ REFERENCE_TIME startTime, _In_ REFERENCE_TIME stopTime, _In_ double rate) override;

	// IQualityControl
	virtual HRESULT _stdcall SetSink(_In_ IQualityControl* qualityControl) override;

	HRESULT _stdcall Run(REFERENCE_TIME startTime);

	bool IsStopped() const { return parent_->IsStopped(); }
	bool IsConnected() const { return pin_ != nullptr; }

	::std::wstring Name() const { return name_; }
	HRESULT SetName(::std::wstring value)
	{
		if (value.length() > MAX_PIN_NAME) return E_FAIL;

		name_ = ::std::move(name_);
	}

	DECLARE_IUNKNOWN;

protected:
	BasePin(_In_ BaseFilter* const filter);

	virtual HRESULT _stdcall QueryInterfaceOverride(IID const& iid, _COM_Outptr_ void** ret);

	virtual HRESULT _stdcall CheckConnection(_In_ IPin* receivePin) = 0;
	virtual HRESULT _stdcall CompleteConnection(_In_ IPin* receivePin) = 0;
	virtual HRESULT _stdcall BreakConnection() = 0;

	virtual HRESULT _stdcall CheckMediaType(_In_ AM_MEDIA_TYPE const& mediaType) = 0;
	virtual HRESULT _stdcall SetMediaType(_In_ AM_MEDIA_TYPE const& mediaType);
	virtual HRESULT _stdcall GetPreferredMediaType(_In_ ULONG position, _Inout_ AM_MEDIA_TYPE* mediaType) = 0;

	HRESULT _stdcall DisconnectInternal();

	HRESULT _stdcall AgreeMediaType(_In_ IPin* receivePin, _In_opt_ AM_MEDIA_TYPE const* mediaType);
	HRESULT _stdcall TryMediaTypes(_In_ IPin* receivePin, _In_opt_ AM_MEDIA_TYPE const* mediaType, _In_ IEnumMediaTypes* enumMediaTypes);
	HRESULT _stdcall TryConnection(_In_ IPin* receivePin, _In_opt_ AM_MEDIA_TYPE const* mediaType);

private:
	BasePin() = delete;
	BasePin(BasePin const&) = delete;
	BasePin(BasePin&&) = delete;

	BasePin& operator=(BasePin const&) = delete;
	BasePin& operator=(BasePin&&) = delete;

protected:
	mutable ::std::recursive_mutex mutex_;
	BaseFilter* const parent_;
	::Microsoft::WRL::ComPtr<IPin> pin_;
	::Microsoft::WRL::ComPtr<IQualityControl> qualityControl_;
	::std::wstring name_;
	MediaType mediaType_;
	bool error_;
	REFERENCE_TIME startTime_, stopTime_;
	double rate_;
};