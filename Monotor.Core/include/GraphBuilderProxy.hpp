#pragma once
#include "DirectShowHelper.hpp"
#include "RunningObjectTableRegistrar.hpp"
#include "FilterProxy.hpp"
#include "MediaControlProxy.hpp"

namespace Mntone {
namespace DirectShowSupport {

class GraphBuilderProxy final
{
public:
	GraphBuilderProxy();

	void Add(FilterProxy const& filter);

	operator IGraphBuilder*() const { return graphBuilder_.Get(); }
	IGraphBuilder* operator->() const { return graphBuilder_.Get(); }

public:
	MediaControlProxy& MediaControl() { return mediaControlProxy_; }

private:
#if _DEBUG
	RunningObjectTableRegistrar tableRegistrar;
#endif
	::Microsoft::WRL::ComPtr<IGraphBuilder> graphBuilder_;
	MediaControlProxy mediaControlProxy_;
};

}
}