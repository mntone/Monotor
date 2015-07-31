#include "pch.hpp"
#include "MediaControlProxy.hpp"

#include "HResultException.hpp"

using namespace Mntone::DirectShowSupport;

void MediaControlProxy::Initialize(IGraphBuilder* graphBuilder)
{
	HRESULT hr = graphBuilder->QueryInterface(mediaControl_.GetAddressOf());
	if (FAILED(hr)) throw HResultException(hr);
}