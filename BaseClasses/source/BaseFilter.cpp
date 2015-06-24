#include "pch.hpp"
#include "BaseFilter.hpp"

using namespace std;

BaseFilter::BaseFilter(CLSID const& clsID)
	: clsID_(clsID)
	, state_(filter_state::stopped)
{ }

HRESULT BaseFilter::QueryInterfaceOverride(IID const& iid, void** ret)
{
	PrintDebugLog(L"Start");
	if (iid == IID_IPersist)
	{
		AddRefInternal();
		*ret = static_cast<IPersist*>(this);
		return S_OK;
	}
	if (iid == IID_IMediaFilter)
	{
		AddRefInternal();
		*ret = static_cast<IMediaFilter*>(this);
		return S_OK;
	}
	if (iid == IID_IBaseFilter)
	{
		AddRefInternal();
		*ret = static_cast<IBaseFilter*>(this);
		return S_OK;
	}
	return UnknownObject::QueryInterfaceOverride(iid, ret);
}

HRESULT BaseFilter::GetClassID(CLSID* clsID)
{
	PrintDebugLog(L"Start");
	CheckPointer(clsID);

	*clsID = clsID_;
	return S_OK;
}

HRESULT BaseFilter::GetState(DWORD /*millisecondsTimeout*/, FILTER_STATE* state)
{
	PrintDebugLog(L"Start");
	CheckPointer(state);

	lock_guard<recursive_mutex> lock(mutex_);
	*state = static_cast<FILTER_STATE>(state_);
	return S_OK;
}

HRESULT BaseFilter::SetSyncSource(IReferenceClock* referenceClock)
{
	PrintDebugLog(L"Start");
	lock_guard<recursive_mutex> lock(mutex_);
	referenceClock_ = referenceClock;

	return S_OK;
}

HRESULT BaseFilter::GetSyncSource(IReferenceClock** referenceClock)
{
	PrintDebugLog(L"Start");
	CheckPointer(referenceClock);

	HRESULT hr = S_OK;
	lock_guard<recursive_mutex> lock(mutex_);
	hr = referenceClock_.CopyTo(referenceClock);
	return hr;
}

HRESULT BaseFilter::QueryFilterInfo(FILTER_INFO* info)
{
	PrintDebugLog(L"Start");
	CheckPointer(info);

	if (name_.empty())
	{
		info->achName[0] = L'\0';
	}
	else
	{
		wcsncpy_s(info->achName, name_.c_str(), name_.length());
	}

	graph_.CopyTo(&info->pGraph);
	return S_OK;
}

HRESULT BaseFilter::JoinFilterGraph(IFilterGraph* graph, LPCWSTR name)
{
	PrintDebugLog(L"Start");
	lock_guard<recursive_mutex> lock(mutex_);

	graph_ = graph;
	if (graph_)
	{
		::Microsoft::WRL::ComPtr<IMediaEventSink> eventSink;
		HRESULT hr = graph_.CopyTo(eventSink.GetAddressOf());
		if (FAILED(hr))
		{
			assert(eventSink == nullptr);
		}
	}

	name_ = name ? name : L"";
	return S_OK;
}

HRESULT BaseFilter::QueryVendorInfo(LPWSTR* /*vendorInfo*/)
{
	PrintDebugLog(L"Start");
	return E_NOTIMPL;
}

HRESULT BaseFilter::NotifyEvent(long eventCode, LONG_PTR param1, LONG_PTR param2)
{
	PrintDebugLog(L"Start");
	::Microsoft::WRL::ComPtr<IMediaEventSink> sink;
	sink = sink_;
	if (sink)
	{
		if (eventCode == EC_COMPLETE) param2 = (LONG_PTR)(IBaseFilter*)this;
		return sink->Notify(eventCode, param1, param2);
	}
	return E_NOTIMPL;
}