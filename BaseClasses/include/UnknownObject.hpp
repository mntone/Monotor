#pragma once

#ifndef IDELEGATEUNKNOWN_DEFINED
DECLARE_INTERFACE(IFalseUnknown)
{
	STDMETHOD(QueryInterfaceOverride) (REFIID, void**) = 0;
	STDMETHOD_(ULONG, AddRefInternal)() = 0;
	STDMETHOD_(ULONG, ReleaseInternal)() = 0;
};
#define IDELEGATEUNKNOWN_DEFINED
#endif

class __declspec(novtable) UnknownObject
	: public IFalseUnknown
{
protected:
	virtual HRESULT _stdcall QueryInterfaceOverride(_In_ IID const& iid, _COM_Outptr_ void** ret) override;
	virtual ULONG _stdcall AddRefInternal() override;
	virtual ULONG _stdcall ReleaseInternal() override;

protected:
	UnknownObject();

private:
	UnknownObject(UnknownObject const&) = delete;
	UnknownObject(UnknownObject&&) = delete;

	UnknownObject& operator=(UnknownObject const&) = delete;
	UnknownObject& operator=(UnknownObject&&) = delete;

protected:
	volatile uint32_t referenceCount_;
};

class __declspec(novtable) DelegateUnknownObject
	: public IFalseUnknown
{
protected:
	virtual HRESULT _stdcall QueryInterfaceOverride(_In_ IID const& iid, _COM_Outptr_ void** ret) override;
	virtual ULONG _stdcall AddRefInternal() override;
	virtual ULONG _stdcall ReleaseInternal() override;

protected:
	DelegateUnknownObject(IUnknown* const target);

private:
	DelegateUnknownObject(DelegateUnknownObject const&) = delete;
	DelegateUnknownObject(DelegateUnknownObject&&) = delete;

	DelegateUnknownObject& operator=(DelegateUnknownObject const&) = delete;
	DelegateUnknownObject& operator=(DelegateUnknownObject&&) = delete;

protected:
	IUnknown* const target_;
};

#define DECLARE_IUNKNOWN \
virtual HRESULT _stdcall QueryInterface(_In_ IID const& iid, _COM_Outptr_ void** ret) override \
{ \
	PrintDebugLog((L"iid: " + GetGuidString(iid)).c_str()); \
	return QueryInterfaceOverride(iid, ret); \
} \
virtual ULONG _stdcall AddRef() override \
{ \
	PrintDebugLog(L"Start"); \
	return AddRefInternal(); \
} \
virtual ULONG _stdcall Release() override \
{ \
	PrintDebugLog(L"Start"); \
	return ReleaseInternal(); \
}