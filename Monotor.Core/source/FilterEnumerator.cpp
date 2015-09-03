#include "pch.hpp"
#include "FilterEnumerator.hpp"

#include "HResultException.hpp"
#include "Variant.hpp"
#include "DirectShowHelper.hpp"

using namespace std;
using namespace Microsoft::WRL;
using namespace Mntone::DirectShowSupport;

FilterEnumerator::FilterEnumerator()
	: majorType_(GUID_NULL)
	, inputMinorType_(GUID_NULL)
	, outputMinorType_(GUID_NULL)
{
	ComPtr<IFilterMapper2> filterMapper;
	CreateFilterMapper(&filterMapper);

	HRESULT hr = filterMapper.CopyTo(&filterMapper_);
	if (FAILED(hr)) throw HResultException(hr);
}

vector<wstring> FilterEnumerator::GetDeviceNames()
{
	GUID inputTypes[2];
	inputTypes[0] = majorType_;
	inputTypes[1] = inputMinorType_;

	GUID outputTypes[2];
	outputTypes[0] = majorType_;
	outputTypes[1] = outputMinorType_;

	ComPtr<IEnumMoniker> enumMoniker;
	HRESULT hr = filterMapper_->EnumMatchingFilters(
		&enumMoniker,
		0,
		TRUE,
		MERIT_DO_NOT_USE,
		TRUE,
		1,
		inputTypes,
		nullptr,
		nullptr,
		FALSE,
		TRUE,
		1,
		outputTypes,
		nullptr,
		nullptr);
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

HRESULT FilterEnumerator::TryGetDeviceMoniker(wstring deviceName, IMoniker** moniker) noexcept
{
	GUID inputTypes[2];
	inputTypes[0] = majorType_;
	inputTypes[1] = inputMinorType_;

	GUID outputTypes[2];
	outputTypes[0] = majorType_;
	outputTypes[1] = outputMinorType_;

	ComPtr<IEnumMoniker> enumMoniker;
	HRESULT hr = filterMapper_->EnumMatchingFilters(
		&enumMoniker,
		0,
		TRUE,
		MERIT_DO_NOT_USE,
		TRUE,
		1,
		inputTypes,
		nullptr,
		nullptr,
		FALSE,
		TRUE,
		1,
		outputTypes,
		nullptr,
		nullptr);
	if (FAILED(hr)) throw HResultException(hr);

	ComPtr<IMoniker> localMoniker;
	while ((hr = enumMoniker->Next(1, &localMoniker, nullptr)) != S_FALSE)
	{
		ComPtr<IPropertyBag> propertyBag;
		hr = localMoniker->BindToStorage(0, 0, IID_IPropertyBag, &propertyBag);
		if (FAILED(hr)) continue;

		ComSupport::VariantString deviceNameVariant;
		hr = propertyBag->Read(constant_friendly_name, deviceNameVariant, 0);
		if (FAILED(hr)) continue;

		if (deviceNameVariant.GetString() == deviceName)
		{
			hr = localMoniker.CopyTo(moniker);
			return hr;
		}
	}
	return E_FAIL;
}