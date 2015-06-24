#include "pch.hpp"
#include "UnknownObject.hpp"

UnknownObject::UnknownObject()
	: referenceCount_(0)
{ }

HRESULT UnknownObject::QueryInterfaceOverride(IID const& iid, void** ret)
{
	PrintDebugLog((L"iid: " + GetClsidString(iid)).c_str());
	if (iid == IID_IUnknown)
	{
		AddRefInternal();
		*ret = reinterpret_cast<IUnknown*>(static_cast<IFalseUnknown*>(this));
		return S_OK;
	}

	*ret = nullptr;
	return E_NOINTERFACE;
}

ULONG UnknownObject::AddRefInternal()
{
	PrintDebugLog((L"current count: " + ::std::to_wstring(referenceCount_)).c_str());
	return InterlockedIncrement(&referenceCount_);
}

ULONG UnknownObject::ReleaseInternal()
{
	PrintDebugLog((L"current count: " + ::std::to_wstring(referenceCount_)).c_str());
	ULONG ret = InterlockedDecrement(&referenceCount_);
	if (ret == 0)
	{
		delete this;
	}
	return ret;
}

DelegateUnknownObject::DelegateUnknownObject(IUnknown* const target)
	: target_(target)
{ }

HRESULT DelegateUnknownObject::QueryInterfaceOverride(IID const& iid, void** ret)
{
	PrintDebugLog((L"iid: " + GetClsidString(iid)).c_str());
	if (iid == IID_IUnknown)
	{
		*ret = reinterpret_cast<IUnknown*>(static_cast<IFalseUnknown*>(this));
		return S_OK;
	}

	*ret = nullptr;
	return E_NOINTERFACE;
}

ULONG DelegateUnknownObject::AddRefInternal()
{
	PrintDebugLog(L"Start");
	return target_->AddRef();
}

ULONG DelegateUnknownObject::ReleaseInternal()
{
	PrintDebugLog(L"Start");
	return target_->Release();
}