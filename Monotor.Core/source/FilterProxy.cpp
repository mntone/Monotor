#include "pch.hpp"
#include "FilterProxy.hpp"

#include "HResultException.hpp"
#include "DirectShowHelper.hpp"

using namespace std;
using namespace Microsoft::WRL;
using namespace Mntone::DirectShowSupport;

bool IsInputPin(PinProxy p) { return p.IsInputPin(); };
bool IsOutputPin(PinProxy p) { return p.IsOutputPin(); };

FilterProxy::FilterProxy(IBaseFilter* baseFilter, ::std::wstring name, bool cacheEnabled) noexcept
	: filter_(baseFilter)
	, name_(name)
	, cacheEnabled_(cacheEnabled)
	, pinCached_(false)
{ }

void FilterProxy::ResetCache() noexcept
{
	pinCached_ = false;
	pins_.clear();
}

vector<PinProxy> FilterProxy::GetPins()
{
	return GetPinsInternal();
}

vector<PinProxy> FilterProxy::GetInputPins()
{
	vector<PinProxy> ret;
	auto pins = GetPinsInternal();
	ret.resize(pins.size());
	auto itr = ::std::copy_if(pins.begin(), pins.end(), ret.begin(), IsInputPin);
	ret.resize(itr - ret.begin());
	return ::std::move(ret);
}

vector<PinProxy> FilterProxy::FindInputPins(::std::wstring id)
{
	vector<PinProxy> ret;
	auto pins = GetPinsInternal();
	ret.resize(pins.size());
	auto itr = ::std::copy_if(pins.begin(), pins.end(), ret.begin(), [id](PinProxy p) { return p.IsInputPin() && id == p.GetId(); });
	ret.resize(itr - ret.begin());
	return ::std::move(ret);
}

vector<PinProxy> FilterProxy::GetOutputPins()
{
	vector<PinProxy> ret;
	auto pins = GetPinsInternal();
	ret.resize(pins.size());
	auto itr = ::std::copy_if(pins.begin(), pins.end(), ret.begin(), IsOutputPin);
	ret.resize(itr - ret.begin());
	return ::std::move(ret);
}

vector<PinProxy> FilterProxy::FindOutputPins(::std::wstring id)
{
	vector<PinProxy> ret;
	auto pins = GetPinsInternal();
	ret.resize(pins.size());
	auto itr = ::std::copy_if(pins.begin(), pins.end(), ret.begin(), [id](PinProxy p) { return p.IsOutputPin() && id == p.GetId(); });
	ret.resize(itr - ret.begin());
	return ::std::move(ret);
}

vector<PinProxy> FilterProxy::GetPinsInternal()
{
	if (pinCached_) return pins_;

	HRESULT hr = S_OK;
	ComPtr<IEnumPins> enumPins;
	hr = filter_->EnumPins(&enumPins);
	if (FAILED(hr)) throw HResultException(hr);

	vector<PinProxy> pins;
	while (true)
	{
		ULONG retrived = 3;
		ComPtr<IPin> pin[3];
		hr = enumPins->Next(3, &pin[0], &retrived);
		if (FAILED(hr)) break;
		for (ULONG i = 0; i < retrived; ++i)
		{
			pins.emplace_back(pin[i].Get());
		}
		if (hr == S_FALSE) break;
	}

	if (!cacheEnabled_) return move(pins);

	pins_ = ::std::move(pins);
	pinCached_ = true;
	return pins_;
}

FilterProxy FilterProxy::CreateInstanceFromClsid(CLSID const& clsid, wstring name, bool cacheEnabled)
{
	ComPtr<IBaseFilter> filter;
	CreateBaseFilter(clsid, &filter);
	return FilterProxy(filter.Get(), name, cacheEnabled);
}

FilterProxy FilterProxy::CreateInsntanceFromMoniker(IMoniker* moniker, wstring name, bool cacheEnabled)
{
	ComPtr<IBaseFilter> filter;
	HRESULT hr = moniker->BindToObject(nullptr, nullptr, IID_IBaseFilter, &filter);
	if (FAILED(hr)) throw HResultException(hr);

	return FilterProxy(filter.Get(), name, cacheEnabled);
}