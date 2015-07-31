#include "pch.hpp"
#include "DeviceEnumerator.hpp"

#include "HResultException.hpp"
#include "Variant.hpp"
#include "DirectShowHelper.hpp"

using namespace std;
using namespace Microsoft::WRL;
using namespace Mntone::DirectShowSupport;

wchar_t const* constant_friendly_name = L"FriendlyName";

DeviceEnumerator::DeviceEnumerator()
{
	ComPtr<ICreateDevEnum> dev_enum;
	CreateSystemDeviceEnum(&dev_enum);

	HRESULT hr = dev_enum.CopyTo(&dev_enum_);
	if (FAILED(hr)) throw HResultException(hr);
}

vector<wstring> DeviceEnumerator::GetDeviceNamesFromCategory(CLSID const& category)
{
	ComPtr<IEnumMoniker> enumMoniker;
	HRESULT hr = dev_enum_->CreateClassEnumerator(category, &enumMoniker, 0);
	if (FAILED(hr)) throw HResultException(hr);

	vector<wstring> deciceInfo;
	ComPtr<IMoniker> moniker;
	while ((hr = enumMoniker->Next(1, &moniker, nullptr)) != S_FALSE)
	{
		ComPtr<IPropertyBag> propertyBag;
		hr = moniker->BindToStorage(0, 0, IID_IPropertyBag, &propertyBag);
		if (FAILED(hr)) continue;

		ComSupport::VariantString deviceNameVariant;
		hr = propertyBag->Read(constant_friendly_name, deviceNameVariant, 0);
		if (FAILED(hr)) continue;

		deciceInfo.emplace_back(deviceNameVariant.GetString());
	}
	return move(deciceInfo);
}

vector<wstring> DeviceEnumerator::GetVideoInputDeviceNames()
{
	return GetDeviceNamesFromCategory(CLSID_VideoInputDeviceCategory);
}

vector<wstring> DeviceEnumerator::GetAudioInputDeviceNames()
{
	return GetDeviceNamesFromCategory(CLSID_AudioInputDeviceCategory);
}

HRESULT DeviceEnumerator::TryGetDeviceMoniker(CLSID const& category, wstring deviceName, IMoniker** moniker) noexcept
{
	ComPtr<IEnumMoniker> enumMoniker;
	HRESULT hr = dev_enum_->CreateClassEnumerator(category, &enumMoniker, 0);
	if (FAILED(hr)) return hr;

	ComPtr<IMoniker> localMoniker;
	while ((hr = enumMoniker->Next(1, &localMoniker, nullptr)) != S_FALSE)
	{
		ComPtr<IPropertyBag> propertyBag;
		hr = localMoniker->BindToStorage(0, 0, IID_IPropertyBag, &propertyBag);
		if (FAILED(hr)) continue;

		ComSupport::VariantString deviceNameVariant;
		hr = propertyBag->Read(L"FriendlyName", deviceNameVariant, 0);
		if (FAILED(hr)) continue;

		if (deviceNameVariant.GetString() == deviceName)
		{
			hr = localMoniker.CopyTo(moniker);
			return hr;
		}
	}
	return E_FAIL;
}

HRESULT DeviceEnumerator::TryGetVideoInputDeviceMoniker(wstring deviceName, IMoniker** moniker) noexcept
{
	return TryGetDeviceMoniker(CLSID_VideoInputDeviceCategory, deviceName, moniker);
}

HRESULT DeviceEnumerator::TryGetAudioInputDeviceMoniker(wstring deviceName, IMoniker** moniker) noexcept
{
	return TryGetDeviceMoniker(CLSID_AudioInputDeviceCategory, deviceName, moniker);
}