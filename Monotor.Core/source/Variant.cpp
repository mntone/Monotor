#include "pch.hpp"
#include "Variant.hpp"

using namespace std;
using namespace Mntone::ComSupport;

Variant::~Variant() noexcept
{
	HRESULT hr = VariantClear(&variant_);
	assert(SUCCEEDED(hr));
}

wstring VariantString::GetString() const noexcept
{
	return wstring(variant_.bstrVal, SysStringLen(variant_.bstrVal));
}