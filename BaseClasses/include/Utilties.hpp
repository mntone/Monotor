#pragma once

#define CheckPointer(__TARGET__) if ((__TARGET__) == nullptr) return E_POINTER;

extern "C" void _PrintDebugLog(wchar_t const* message, char const* func, char const* filename, int line);
#if _DEBUG
#define PrintDebugLog(__MESSAGE__) _PrintDebugLog(__MESSAGE__, __FUNCSIG__, __FILE__, __LINE__)
#else
#define PrintDebugLog(__MESSAGE__) 0
#endif

::std::wstring GetGuidString(GUID guid);
::std::wstring GetAssemblyName(HINSTANCE hinstance);

extern "C" {

	HRESULT _stdcall CreateMemoryAllocator(_Outptr_opt_ IMemAllocator*& memAllocator);
	HRESULT _stdcall GetGlobalString(::std::wstring source, wchar_t** ret);
	HRESULT _stdcall SetupRegisterServer(
		CLSID clsid,
		::std::wstring description,
		::std::wstring filename,
		::std::wstring threadingModel = L"Both",
		::std::wstring serverType = L"InprocServer32");
	HRESULT _stdcall SetupUnregisterServer(CLSID clsid);

}