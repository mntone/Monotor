#include "pch.hpp"
#include "dbgstring.hpp"

template<> static void basic_debug_streambuf<char>::output_debug_string(char const* value) { OutputDebugStringA(value); }
template<> static void basic_debug_streambuf<wchar_t>::output_debug_string(wchar_t const* value) { OutputDebugStringW(value); }

dstream cdbg;
wdstream wcdbg;