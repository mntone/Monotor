#pragma once
#include "DeviceEnumerator.hpp"
#include "FilterEnumerator.hpp"
#include "GraphBuilderProxy.hpp"

namespace Mntone {
namespace Monotor {

class GraphBuilder final
{
public:
	void Run();

	DirectShowSupport::FilterProxy ConnectForVideo(::std::wstring deviceName, MediaType& mediaType);
	DirectShowSupport::FilterProxy ConnectForAudio(::std::wstring deviceName, MediaType& mediaType);

private:
	DirectShowSupport::FilterProxy ConnectSmartTee(DirectShowSupport::FilterProxy& sourceFilter, MediaType& mediaType);
	DirectShowSupport::PinProxy GetSmartTeePreviewOrDefault(DirectShowSupport::FilterProxy& targetFilter);

	DirectShowSupport::FilterProxy ConnectForMjpeg(DirectShowSupport::FilterProxy& sourceFilter, MediaType& mediaType);
	DirectShowSupport::FilterProxy ConnectForYuv(DirectShowSupport::FilterProxy& sourceFilter, MediaType& mediaType);
	DirectShowSupport::FilterProxy ConnectEvrRenderer(DirectShowSupport::FilterProxy& lastTransformFilter);

	DirectShowSupport::FilterProxy ConnectForSurround(DirectShowSupport::FilterProxy& sourceFilter, MediaType& mediaType);
	DirectShowSupport::FilterProxy ConnectWaveOutRenderer(DirectShowSupport::FilterProxy& lastTransformFilter);

private:
	DirectShowSupport::DeviceEnumerator enumrator_;
	DirectShowSupport::FilterEnumerator filterEnumrator_;
	DirectShowSupport::GraphBuilderProxy graphBuilderProxy_;
};

}
}