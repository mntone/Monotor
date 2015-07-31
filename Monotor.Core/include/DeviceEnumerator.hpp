#pragma once

namespace Mntone {
namespace DirectShowSupport {

class DeviceEnumerator final
{
public:
	DeviceEnumerator();

	::std::vector<::std::wstring> GetDeviceNamesFromCategory(CLSID const& category);
	::std::vector<::std::wstring> GetVideoInputDeviceNames();
	::std::vector<::std::wstring> GetAudioInputDeviceNames();

	HRESULT TryGetDeviceMoniker(CLSID const& category, ::std::wstring deviceName, IMoniker** moniker) noexcept;
	HRESULT TryGetVideoInputDeviceMoniker(::std::wstring deviceName, IMoniker** moniker) noexcept;
	HRESULT TryGetAudioInputDeviceMoniker(::std::wstring deviceName, IMoniker** moniker) noexcept;

private:
	::Microsoft::WRL::ComPtr<ICreateDevEnum> dev_enum_;
};

}
}