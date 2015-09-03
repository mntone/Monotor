#pragma once

namespace Mntone {
namespace DirectShowSupport {

class FilterEnumerator final
{
public:
	FilterEnumerator();

	::std::vector<::std::wstring> GetDeviceNames();

	HRESULT TryGetDeviceMoniker(::std::wstring deviceName, IMoniker** moniker) noexcept;

public:
	void SetMajorType(GUID value) { majorType_ = value; }
	void SetInputMinorType(GUID value) { inputMinorType_ = value; }
	void SetOutputMinorType(GUID value) { outputMinorType_ = value; }

private:
	::Microsoft::WRL::ComPtr<IFilterMapper2> filterMapper_;

	GUID majorType_, inputMinorType_, outputMinorType_;
};

}
}