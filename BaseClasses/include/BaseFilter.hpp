#pragma once
#include "UnknownObject.hpp"

class __declspec(novtable) BaseFilter
	: public UnknownObject
	, public IBaseFilter
{
	friend class BasePin;
	friend class BaseInputPin;
	friend class BaseOutputPin;

public:
	// IPersist
	virtual HRESULT _stdcall GetClassID(_Out_ CLSID* clsID) override;

	// IMediaFilter
	virtual HRESULT _stdcall GetState(_In_ DWORD millisecondsTimeout, _Out_ FILTER_STATE* state) override final;
	virtual HRESULT _stdcall SetSyncSource(_In_opt_ IReferenceClock* referenceClock) override final;
	virtual HRESULT _stdcall GetSyncSource(_Outptr_result_maybenull_ IReferenceClock** referenceClock) override final;

	// IBaseFilter
	virtual HRESULT _stdcall QueryFilterInfo(_Out_ FILTER_INFO* info) override final;
	virtual HRESULT _stdcall JoinFilterGraph(_In_opt_ IFilterGraph* graph, _In_opt_ LPCWSTR name) override final;
	virtual HRESULT _stdcall QueryVendorInfo(_Out_ LPWSTR* vendorInfo) override final;

	bool IsActive() const { return state_ != filter_state::stopped; }
	bool IsStopped() const { return state_ == filter_state::stopped; }

	::std::wstring Name() const { return name_; }
	HRESULT SetName(::std::wstring value)
	{
		if (value.length() > MAX_FILTER_NAME) return E_FAIL;

		name_ = ::std::move(name_);
	}

	DECLARE_IUNKNOWN;

protected:
	BaseFilter(_In_ CLSID const& clsID);

	virtual HRESULT _stdcall QueryInterfaceOverride(_In_ IID const& iid, _COM_Outptr_ void** ret) override;

	HRESULT _stdcall NotifyEvent(_In_ long eventCode, _In_ LONG_PTR param1, _In_ LONG_PTR param2);

private:
	BaseFilter() = delete;
	BaseFilter(BaseFilter const&) = delete;
	BaseFilter(BaseFilter&&) = delete;

	BaseFilter& operator=(BaseFilter const&) = delete;
	BaseFilter& operator=(BaseFilter&&) = delete;

protected:
	mutable ::std::recursive_mutex mutex_;
	CLSID const& clsID_;
	enum class filter_state : uint8_t
	{
		stopped,
		paused,
		running,
	} state_;
	::Microsoft::WRL::ComPtr<IReferenceClock> referenceClock_;
	::Microsoft::WRL::ComPtr<IFilterGraph> graph_;
	::Microsoft::WRL::ComPtr<IMediaEventSink> sink_;
	::std::wstring name_;
};