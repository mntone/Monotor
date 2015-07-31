#include "pch.hpp"
#include "ComInitializeWrapper.hpp"

#include "HResultException.hpp"

using namespace Mntone::ComSupport;

ComInitializeWrapper::ComInitializeWrapper(DWORD flags)
{
	HRESULT hr = CoInitializeEx(nullptr, flags);
	if (FAILED(hr)) throw HResultException(hr);
}

ComInitializeWrapper::~ComInitializeWrapper()
{
	CoUninitialize();
}