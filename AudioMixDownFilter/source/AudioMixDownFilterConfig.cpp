#include "pch.hpp"
#include "AudioMixDownFilterDefine.hpp"
#include "AudioMixDownFilter.hpp"
#include "../AudioMixDownFilter_i.c"

namespace
{

constexpr REGPINTYPES regPinTypes[] =
{
	{
		&MEDIATYPE_Audio,
		&MEDIASUBTYPE_PCM,
	},
};

constexpr REGFILTERPINS2 registrationPin[] =
{
	{
		0,
		1,
		_ARRAYSIZE(regPinTypes),
		regPinTypes,
		0,
		nullptr,
		&GUID_NULL,
	},
	{
		REG_PINFLAG_B_OUTPUT,
		1,
		_ARRAYSIZE(regPinTypes),
		regPinTypes,
		0,
		nullptr,
		&GUID_NULL,
	},
};

REGFILTER2 const registrationFilter =
{
	2,
	MERIT_DO_NOT_USE,
	_ARRAYSIZE(registrationPin),
	reinterpret_cast<REGFILTERPINS const*>(&registrationPin),
};

}

::std::vector<::std::pair<CLSID const&, ::std::function<HRESULT(ClassUnloadContext, IUnknown**)>>> g_Classes =
{
	{ CLSID_AudioMixDownFilter, AudioMixDownFilter::CreateInstance }
};

extern "C"
{
	HRESULT _stdcall DllRegisterServer()
	{
#if _DEBUG
		// attach point
		MessageBoxW(nullptr, L"Attach this process if you debug.", L"Check", MB_OK);
#endif

		HRESULT hr = SetupRegisterServer(CLSID_AudioMixDownFilter, NAME_AudioMixDownFilter, GetAssemblyName(g_hInst));
		assert(SUCCEEDED(hr));

		{
			::Microsoft::WRL::ComPtr<IFilterMapper2> filterMapper2;
			hr = CoCreateInstance(CLSID_FilterMapper2, nullptr, CLSCTX_INPROC_SERVER, IID_IFilterMapper2, reinterpret_cast<void**>(filterMapper2.GetAddressOf()));
			if (SUCCEEDED(hr))
			{
				hr = filterMapper2->RegisterFilter(
					CLSID_AudioMixDownFilter,
					NAME_AudioMixDownFilter,
					nullptr,
					nullptr,
					nullptr,
					&registrationFilter);
			}
		}
		CoUninitialize();
		return hr;
	}

	HRESULT _stdcall DllUnregisterServer()
	{
#if _DEBUG
		// attach point
		MessageBoxW(nullptr, L"Attach this process if you debug.", L"Check", MB_OK);
#endif

		HRESULT hr = SetupUnregisterServer(CLSID_AudioMixDownFilter);
		assert(SUCCEEDED(hr));

		{
			::Microsoft::WRL::ComPtr<IFilterMapper2> filterMapper2;
			hr = CoCreateInstance(CLSID_FilterMapper2, nullptr, CLSCTX_INPROC_SERVER, IID_IFilterMapper2, reinterpret_cast<void**>(filterMapper2.GetAddressOf()));
			if (SUCCEEDED(hr))
			{
				hr = filterMapper2->UnregisterFilter(nullptr, nullptr, CLSID_AudioMixDownFilter);
			}
		}
		CoUninitialize();
		return hr;
	}
}