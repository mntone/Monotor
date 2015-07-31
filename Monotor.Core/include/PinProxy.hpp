#pragma once
#include "MediaType.hpp"

namespace Mntone {
namespace DirectShowSupport {

class PinProxy final
{
public:
	PinProxy(bool cacheEnabled = true) noexcept;
	PinProxy(IPin* pin, bool cacheEnabled = true) noexcept;

	void ResetCache() noexcept;

	bool IsInputPin() { return GetDirectionInternal() == PIN_DIRECTION::PINDIR_INPUT; }
	bool IsOutputPin() { return GetDirectionInternal() == PIN_DIRECTION::PINDIR_OUTPUT; }

	::std::wstring GetId();
	::std::vector<MediaType> GetPreferedMediaType();

	operator IPin*() const { return pin_.Get(); }
	IPin* operator->() const { return pin_.Get(); }

private:
	PIN_DIRECTION GetDirectionInternal();

private:
	::Microsoft::WRL::ComPtr<IPin> pin_;

	struct
	{
		bool cacheEnabled_ : 1;
		bool directionCached_ : 1;
		bool idCached_ : 1;
		bool mediaTypeCached_ : 1;
	};
	PIN_DIRECTION direction_;
	::std::wstring id_;
	::std::vector<MediaType> mediaTypes_;
};

}
}