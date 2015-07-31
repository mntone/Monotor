#pragma once

namespace Mntone {
namespace DirectShowSupport {

class MediaControlProxy final
{
	friend class GraphBuilderProxy;

public:
	IMediaControl* operator->() const { return mediaControl_.Get(); }

private:
	MediaControlProxy() = default;
	MediaControlProxy(MediaControlProxy const&) = delete;
	MediaControlProxy(MediaControlProxy&&) = delete;

	MediaControlProxy& operator=(MediaControlProxy const&) = delete;
	MediaControlProxy& operator=(MediaControlProxy&&) = delete;

	void Initialize(IGraphBuilder* graphBuilder);

private:
	::Microsoft::WRL::ComPtr<IMediaControl> mediaControl_;
};

}
}