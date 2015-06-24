#include "pch.hpp"
#include "BasePin.hpp"

using namespace std;

BasePin::BasePin(BaseFilter* const filter)
	: DelegateUnknownObject(reinterpret_cast<IUnknown*>(static_cast<IFalseUnknown*>(filter)))
	, parent_(filter)
	, error_(false)
	, rate_(1.0)
{ }

HRESULT BasePin::QueryInterfaceOverride(IID const& iid, _COM_Outptr_ void** ret)
{
	if (iid == IID_IPin)
	{
		AddRefInternal();
		*ret = static_cast<IPin*>(this);
		return S_OK;
	}
	if (iid == IID_IQualityControl)
	{
		AddRefInternal();
		*ret = static_cast<IQualityControl*>(this);
		return S_OK;
	}
	*ret = nullptr;
	return E_NOINTERFACE;
}

HRESULT BasePin::QueryPinInfo(PIN_INFO* info)
{
	CheckPointer(info);

	if (name_.empty())
	{
		info->achName[0] = L'\0';
	}
	else
	{
		wcsncpy_s(info->achName, name_.c_str(), name_.length());
	}

	info->pFilter = parent_;
	if (parent_ != nullptr)
	{
		parent_->AddRef();
	}

	return S_OK;
}

HRESULT BasePin::QueryId(LPWSTR* id)
{
	return GetGlobalString(name_, id);
}

HRESULT BasePin::QueryAccept(AM_MEDIA_TYPE const* mediaType)
{
	CheckPointer(mediaType);

	HRESULT hr = CheckMediaType(*mediaType);
	if (FAILED(hr)) return S_FALSE;
	return hr;
}

HRESULT BasePin::QueryInternalConnections(IPin** /*pin*/, ULONG* /*pinCount*/)
{
	return E_NOTIMPL;
}

HRESULT BasePin::NewSegment(REFERENCE_TIME startTime, REFERENCE_TIME stopTime, double rate)
{
	startTime_ = startTime;
	stopTime_ = stopTime;
	rate_ = rate;
	return S_OK;
}

HRESULT BasePin::SetSink(_In_ IQualityControl* qualityControl)
{
	lock_guard<recursive_mutex> lock(mutex_);
	qualityControl_ = qualityControl;
	return S_OK;
}

HRESULT BasePin::Run(REFERENCE_TIME /*startTime*/)
{
	return S_OK;
}