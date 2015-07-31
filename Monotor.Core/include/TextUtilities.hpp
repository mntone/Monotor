#pragma once

namespace Mntone {
namespace Monotor {

template<typename charT>
class basic_string_reference
{
public:
	using pointer = charT*;
	using const_pointer = charT const*;
	using size_type = size_t;

	basic_string_reference() noexcept
	{ }

	basic_string_reference(const_pointer ptr, size_type length) noexcept
		: begin_(ptr), end_(ptr + length)
	{ }

	basic_string_reference(const_pointer begin, const_pointer end) noexcept
		: begin_(begin), end_(end)
	{ }

	::std::basic_string<charT> to_string() noexcept
	{
		return ::std::basic_string<charT>(begin_, end_);
	}

	template<typename integer>
	integer to_integer() noexcept;

private:
	const_pointer begin_, end_;
};
using wstring_reference = basic_string_reference<wchar_t>;

template<>
template<typename integer>
integer basic_string_reference<wchar_t>::to_integer() noexcept
{
	auto str = to_string();
	return static_cast<integer>(_wtoi64(str.c_str()));
}

::std::vector<wstring_reference> Split(::std::wstring const& str, wchar_t delimiter = L',')
{
	::std::vector<wstring_reference> ret;
	auto current = str.data();
	auto end = current + str.length();
	for (auto ptr = current; ptr != end; ++ptr)
	{
		if (*ptr == delimiter)
		{
			if (ptr != current)
			{
				ret.emplace_back(current, ptr);
				current = ptr + 1;
			}
			else
			{
				current = ptr + 1;
			}
		}
	}
	if (current != end)
	{
		ret.emplace_back(current, end);
	}
	return ::std::move(ret);
}

}
}