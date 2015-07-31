#pragma once

namespace Mntone {
namespace DirectShowSupport {

#if _DEBUG
class RunningObjectTableRegistrar final
{
public:
	RunningObjectTableRegistrar() noexcept;
	RunningObjectTableRegistrar(IGraphBuilder* graphBuilder);

	~RunningObjectTableRegistrar() noexcept;

	void Initialize(IGraphBuilder* graphBuilder);

private:
	bool initialized_;
	::Microsoft::WRL::ComPtr<IRunningObjectTable> runningObjectTable_;
	DWORD registerID_;
};
#endif

}
}