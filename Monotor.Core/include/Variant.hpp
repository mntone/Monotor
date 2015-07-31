#pragma once

namespace Mntone {
namespace ComSupport {

class Variant
{
public:
	Variant() noexcept { }
	~Variant() noexcept;

	operator VARIANT*() { return &variant_; }

protected:
	VARIANT variant_;
};

class VariantString final
	: public Variant
{
public:
	VariantString() noexcept : Variant() { variant_.vt = VT_BSTR; }

	::std::wstring GetString() const noexcept;
};

}
}