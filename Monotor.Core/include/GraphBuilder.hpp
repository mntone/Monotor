#pragma once
#include "DeviceEnumerator.hpp"
#include "GraphBuilderProxy.hpp"

namespace Mntone {
namespace Monotor {

class GraphBuilder final
{
public:
	void Run();

	DirectShowSupport::FilterProxy ConnectForVideo(::std::wstring deviceName, MediaType& mediaType);

private:
	DirectShowSupport::FilterProxy ConnectForMjpeg(DirectShowSupport::FilterProxy& sourceFilter, MediaType& mediaType);
	DirectShowSupport::FilterProxy ConnectForEvrRenderer(DirectShowSupport::FilterProxy& lastTransformFilter);

private:
	DirectShowSupport::DeviceEnumerator enumrator_;
	DirectShowSupport::GraphBuilderProxy graphBuilderProxy_;
};

}
}