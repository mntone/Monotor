#pragma once

#include "targetver.hpp"

#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <cassert>

#include <algorithm>
#include <numeric>
#include <memory>
#include <iterator>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <sstream>
#include <dbgstring.hpp>

#if _DEBUG
inline _VCRT_ALLOCATOR void* __CRTDECL operator new(
	_In_   size_t      _Size,
	::std::nothrow_t const&,
	_In_   int         _BlockUse,
	_In_z_ char const* _FileName,
	_In_   int         _LineNumber)
{
	try
	{
		return operator new(_Size, _BlockUse, _FileName, _LineNumber);
	}
	catch(::std::bad_alloc&)
	{
		return nullptr;
	}
}
#define new(__NOTHROW__) new(__NOTHROW__, _NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include <winapifamily.h>
#define WIN32_LEAN_AND_MEAN
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSERVICE
#define NOSOUND
#define NOCOMM
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#include <windows.h>
#include <wrl\client.h>
#include <dshow.h>
#include <Dvdmedia.h>
#include <evr.h>

#define CheckPointer(__TARGET__) if ((__TARGET__) == nullptr) return E_POINTER;