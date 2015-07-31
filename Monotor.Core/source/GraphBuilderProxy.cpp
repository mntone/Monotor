#include "pch.hpp"
#include "GraphBuilderProxy.hpp"

#include "HResultException.hpp"

using namespace Mntone::DirectShowSupport;

GraphBuilderProxy::GraphBuilderProxy()
{
	CreateGraphBuilder(&graphBuilder_);
	mediaControlProxy_.Initialize(graphBuilder_.Get());

#if _DEBUG
	tableRegistrar.Initialize(graphBuilder_.Get());
#endif
}

void GraphBuilderProxy::Add(FilterProxy const& filter)
{
	HRESULT hr = graphBuilder_->AddFilter(filter, filter.Name().c_str());
	if (FAILED(hr)) throw HResultException(hr);
}