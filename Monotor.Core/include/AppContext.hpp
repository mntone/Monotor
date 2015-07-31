#pragma once
#include "parameters.hpp"
#include "DeviceEnumerator.hpp"
#include "MediaType.hpp"
#include "GraphBuilder.hpp"

namespace Mntone {
namespace Monotor {

class GraphBuilder;
class AppContext final
{
public:
	void Initialize();
	void ChangingDevice();

	void Start();

private:
	void OnVideoDeviceChanged();
	void OnAudioDeviceChanged();

public:
	::mnfx::typed_event_handler<AppContext, ::mnfx::event_args> const& GraphStarted() const noexcept { return graphStarted_; }
	::mnfx::typed_event_handler<AppContext, ::mnfx::event_args> const& VideoDeviceChanged() const noexcept { return videoDeviceChanged_; }
	::mnfx::typed_event_handler<AppContext, ::mnfx::event_args> const& VideoMediaTypeChanged() const noexcept { return videoMediaTypeChanged_; }
	::mnfx::typed_event_handler<AppContext, ::mnfx::event_args> const& AudioDeviceChanged() const noexcept { return audioDeviceChanged_; }
	::mnfx::typed_event_handler<AppContext, ::mnfx::event_args> const& AudioMediaTypeChanged() const noexcept { return audioMediaTypeChanged_; }

public:
	::std::vector<::std::wstring> VideoDevices() const noexcept { return videoDevices_; }
	::std::vector<MediaType> const& VideoMediaTypes() const noexcept { return videoMediaTypes_; }
	::std::vector<::std::wstring> AudioDevices() const noexcept { return audioDevices_; }
	::std::vector<MediaType> const& AudioMediaTypes() const noexcept { return audioMediaTypes_; }

	::std::wstring GetSelectedVideoDevice() const noexcept { return selectedVideoDevice_; }
	void SetSelectedVideoDevice(::std::wstring value) noexcept;

	::std::wstring GetSelectedAudioDevice() const noexcept { return selectedAudioDevice_; }
	void SetSelectedAudioDevice(::std::wstring value) noexcept;

	::std::pair<::std::int16_t, ::std::int16_t> GetHeightAndWidth() const noexcept { return ::std::make_pair(height_, width_); }
	void SetHeightAndWidth(::std::pair<::std::int16_t, ::std::int16_t> value) noexcept
	{
		height_ = value.first;
		width_ = value.second;
	}

	::std::int8_t GetBit() const noexcept { return bit_; }
	void SetBit(::std::int8_t value) noexcept { bit_ = value; }

	::std::pair<::std::int16_t, ::std::int16_t> GetAspect() const noexcept { return ::std::make_pair(aspectY_, aspectX_); }
	void SetAspect(::std::pair<::std::int16_t, ::std::int16_t> value) noexcept
	{
		aspectY_ = value.first;
		aspectX_ = value.second;
	}

	float GetFramerate() const noexcept { return framerate_; }
	void SetFramerate(float value) noexcept { framerate_ = value; }

	::Microsoft::WRL::ComPtr<IMFVideoDisplayControl> GetVideoDisplayControl() const noexcept { return videoDisplayControl_; }

private:
	DirectShowSupport::DeviceEnumerator enumrator;

	::std::vector<::std::wstring> videoDevices_, audioDevices_;
	::std::vector<MediaType> videoMediaTypes_, audioMediaTypes_;
	::std::wstring selectedVideoDevice_, selectedAudioDevice_;

	::std::int8_t bit_;
	::std::int16_t height_, width_, aspectY_, aspectX_;
	float framerate_;

	::Microsoft::WRL::ComPtr<IMFVideoDisplayControl> videoDisplayControl_;
	::std::unique_ptr<GraphBuilder> builder_;

	::mnfx::typed_event_handler<AppContext, ::mnfx::event_args> graphStarted_, videoDeviceChanged_, videoMediaTypeChanged_, audioDeviceChanged_, audioMediaTypeChanged_;
};

}
}