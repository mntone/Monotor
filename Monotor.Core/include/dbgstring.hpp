#pragma once
#include <windows.h>
#include <cstdlib>
#include <iostream>

template<class charT, class traits = ::std::char_traits<charT>>
class basic_debug_streambuf
	: public ::std::basic_streambuf<charT, traits>
{
public:
	basic_debug_streambuf() : current_size(init_size), buffer(nullptr)
	{
		setbuf(0, 0);
	}

	virtual ~basic_debug_streambuf()
	{
		sync();
		free(buffer);
	}

protected:
	::std::streampos seekoff(::std::streamoff off, ::std::ios::seek_dir dir, int mode = ::std::ios::in | ::std::ios::out) { return traits::eof(); }
	::std::streampos seekpos(::std::streampos pos, int mode = ::std::ios::in | ::std::ios::out) { return traits::eof(); }
	int_type overflow(int_type ch = traits::eof())
	{
		charT buffer[2] = { 0 };
		buffer[0] = static_cast<charT>(ch);
		output_debug_string(buffer);
		return 0;
	}
	int_type underflow() { return traits::eof(); }

	static void output_debug_string(charT const* value);

private:
	static const int init_size = 0x100;
	int current_size;
	charT* buffer;
};

template<class charT, class traits = ::std::char_traits<charT>>
class basic_debug_stream : public ::std::basic_ostream<charT, traits>
{
public:
	basic_debug_stream()
		: std::basic_ostream<charT, traits>(debug_streambuf_p = new basic_debug_streambuf<charT, traits>())
	{ }

	virtual ~basic_debug_stream() { delete debug_streambuf_p; }

private:
	basic_debug_streambuf<charT, traits> *debug_streambuf_p;
};

using dstreambuf = basic_debug_streambuf<char>;
using dstream = basic_debug_stream<char>;

using wdstreambuf = basic_debug_streambuf<wchar_t>;
using wdstream = basic_debug_stream<wchar_t>;

extern dstream cdbg;
extern wdstream wcdbg;