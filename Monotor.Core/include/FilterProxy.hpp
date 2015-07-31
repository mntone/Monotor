#pragma once
#include "PinProxy.hpp"

namespace Mntone {
namespace DirectShowSupport {

class FilterProxy final
{
public:
	FilterProxy(IBaseFilter* baseFilter, ::std::wstring name, bool cacheEnabled = true) noexcept;

	void ResetCache() noexcept;

	::std::vector<PinProxy> GetPins();
	::std::vector<PinProxy> GetInputPins();
	::std::vector<PinProxy> FindInputPins(::std::wstring id);
	::std::vector<PinProxy> GetOutputPins();
	::std::vector<PinProxy> FindOutputPins(::std::wstring id);

	operator IBaseFilter*() const { return filter_.Get(); }
	IBaseFilter* operator->() const { return filter_.Get(); }

	static FilterProxy CreateInstanceFromClsid(CLSID const& clsid, ::std::wstring name, bool cacheEnabled = true);
	static FilterProxy CreateInsntanceFromMoniker(IMoniker* moniker, ::std::wstring name, bool cacheEnabled = true);

private:
	::std::vector<PinProxy> GetPinsInternal();

public:
	::std::wstring Name() const noexcept { return name_; }

private:
	::std::wstring name_;
	::Microsoft::WRL::ComPtr<IBaseFilter> filter_;

	struct
	{
		bool cacheEnabled_ : 1;
		bool pinCached_ : 1;
	};
	::std::vector<PinProxy> pins_;
};

}
}