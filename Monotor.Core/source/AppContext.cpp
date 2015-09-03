#include "pch.hpp"
#include "AppContext.hpp"

#include "FilterProxy.hpp"
#include "HResultException.hpp"
#include "AppException.hpp"

using namespace std;
using namespace Microsoft::WRL;
using namespace Mntone::DirectShowSupport;
using namespace Mntone::Monotor;

void AppContext::Initialize()
{
	ChangingDevice();
}

void AppContext::ChangingDevice()
{
	videoDevices_ = enumrator.GetVideoInputDeviceNames();
	videoDeviceChanged_.invoke(*this, mnfx::event_args());

	audioDevices_ = enumrator.GetAudioInputDeviceNames();
	audioDeviceChanged_.invoke(*this, mnfx::event_args());

#if _DEBUG
	wcdbg << L"Device changed." << endl;
	wcdbg << L"---[ Video Devices ]-------------" << endl;
	copy(videoDevices_.begin(), videoDevices_.end(), ostream_iterator<wstring, wchar_t>(wcdbg, L"\n"));
	wcdbg << L"---[ Audio Devices ]-------------" << endl;
	copy(audioDevices_.begin(), audioDevices_.end(), ostream_iterator<wstring, wchar_t>(wcdbg, L"\n"));
#endif
}

void AppContext::Start()
{
	HRESULT hr = S_OK;

	if (none_of(videoDevices_.cbegin(), videoDevices_.cend(), [=](wstring name) { return name == selectedVideoDevice_; }))
	{
		throw AppException(L"Cannot find video device.");
	}

	MediaType videoMediaType;
	if (memcmp(fourcc_, "MJPG", 4) == 0)
	{
		hr = CreateMjpegVideoType(height_, width_, framerate_, videoMediaType.mt_);
	}	
	else if (memcmp(fourcc_, "YUY2", 4) == 0 || memcmp(fourcc_, "UYVY", 4) == 0 || memcmp(fourcc_, "HDYC", 4) == 0)
	{
		hr = CreateYuvVideoType(height_, width_, framerate_, fourcc_, videoMediaType.mt_);
	}
	else
	{
		throw AppException(L"Unknown format.");
	}
	if (FAILED(hr)) throw HResultException(hr);

	MediaType audioMediaType;
	bool isAudioEnabled = false;
	if (any_of(audioDevices_.cbegin(), audioDevices_.cend(), [=](wstring name) { return name == selectedAudioDevice_; }))
	{
		hr = CreatePcmAudioType(channel_, sampleRate_, audioBit_, audioMediaType.mt_);
		if (SUCCEEDED(hr))
		{
			isAudioEnabled = true;
		}
	}

	builder_ = make_unique<GraphBuilder>();
	try
	{
		auto videoRendererFilter = builder_->ConnectForVideo(selectedVideoDevice_, videoMediaType);
		if (isAudioEnabled)
		{
			auto audioRendererFilter = builder_->ConnectForAudio(selectedAudioDevice_, audioMediaType);
		}
		builder_->Run();

		ComPtr<IMFVideoDisplayControl> videoDisplayControl;
		DirectShowSupport::GetVideoDisplayControl(videoRendererFilter, &videoDisplayControl);

		hr = videoDisplayControl.CopyTo(&videoDisplayControl_);
		if (FAILED(hr)) throw HResultException(hr);
		
		graphStarted_.invoke(*this, ::mnfx::event_args());
	}
	catch (HResultException) { builder_.reset(); throw; }
	catch (AppException) { builder_.reset(); throw; }
}

void AppContext::OnVideoDeviceChanged()
{
	try
	{
		ComPtr<IMoniker> moniker;
		HRESULT hr = enumrator.TryGetVideoInputDeviceMoniker(selectedVideoDevice_, &moniker);
		if (SUCCEEDED(hr))
		{
			auto filter = FilterProxy::CreateInsntanceFromMoniker(moniker.Get(), selectedVideoDevice_);
			auto outputPins = filter.GetOutputPins();
			videoMediaTypes_ = outputPins[0].GetPreferedMediaType();

			videoMediaTypeChanged_.invoke(*this, ::mnfx::event_args());
		}
		else
		{
			videoMediaTypes_.clear();
		}
	}
	catch (HResultException ex)
	{
	}
}

void AppContext::OnAudioDeviceChanged()
{
	try
	{
		ComPtr<IMoniker> moniker;
		HRESULT hr = enumrator.TryGetAudioInputDeviceMoniker(selectedAudioDevice_, &moniker);
		if (SUCCEEDED(hr))
		{
			auto filter = FilterProxy::CreateInsntanceFromMoniker(moniker.Get(), selectedVideoDevice_);
			auto outputPins = filter.GetOutputPins();
			audioMediaTypes_ = outputPins[0].GetPreferedMediaType();

			audioMediaTypeChanged_.invoke(*this, ::mnfx::event_args());
		}
		else
		{
			audioMediaTypes_.clear();
		}
	}
	catch (HResultException ex)
	{
	}
}


void AppContext::SetSelectedVideoDevice(::std::wstring value) noexcept
{
	if (selectedVideoDevice_ == value) return;
	selectedVideoDevice_ = value;

	OnVideoDeviceChanged();
}

void AppContext::SetSelectedAudioDevice(::std::wstring value) noexcept
{
	if (selectedAudioDevice_ == value) return;
	selectedAudioDevice_ = value;

	OnAudioDeviceChanged();
}