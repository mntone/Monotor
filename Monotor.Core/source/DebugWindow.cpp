#include "pch.hpp"
#include "DebugWindow.hpp"

#include "TextUtilities.hpp"
#include "layouts.hpp"
#include "combo_box_base.hpp"
#include "label.hpp"
#include "collections_impl.hpp"

#include "AppException.hpp"

using namespace std;
using namespace mnfx;
using namespace Mntone::Monotor;

DebugWindow::DebugWindow()
	: window()
{
	set_text(L"Monotor Debug Window");
	set_left(48);
	set_height(360);
	set_width(960);

	size_t dummy = 0;
	evrPanel_ = new EvrPanel();
	videoDevicesBox_ = new dropdown_box();
	videoDevicesBox_->select().add([&](combo_box_base const& s, value_change_event_args<wstring> e)
	{
		ctx_.SetSelectedVideoDevice(e.new_value);
		return S_OK;
	}, dummy);
	videoMediaTypesBox_ = new combo_box();
	videoMediaTypesBox_->select().add(bind(&DebugWindow::VideoMediaTypeUpdateCallback, this, placeholders::_1, placeholders::_2), dummy);
	audioDevicesBox_ = new dropdown_box();
	audioDevicesBox_->select().add([&](combo_box_base const& s, value_change_event_args<wstring> e)
	{
		ctx_.SetSelectedAudioDevice(e.new_value);
		return S_OK;
	}, dummy);
	audioMediaTypesBox_ = new combo_box();
	audioMediaTypesBox_->select().add(bind(&DebugWindow::AudioMediaTypeUpdateCallback, this, placeholders::_1, placeholders::_2), dummy);
	startButton_ = new button(L"Preview");
	startButton_->click().add(bind(&DebugWindow::StartClickCallback, this, placeholders::_1, placeholders::_2), dummy);
	auto settings = new grid(
	{ 28, 7, 28, 9, 28, 7, 28, 9, 28, grid_length() },
	{ 60, 7, grid_length() },
	{
		make_tuple(0, 0, new label(L"&video: ")),
		make_tuple(0, 2, videoDevicesBox_),
		make_tuple(2, 0, new label(L" &format: ")),
		make_tuple(2, 2, videoMediaTypesBox_),
		make_tuple(4, 0, new label(L"&audio: ")),
		make_tuple(4, 2, audioDevicesBox_),
		make_tuple(6, 0, new label(L" f&ormat: ")),
		make_tuple(6, 2, audioMediaTypesBox_),
		make_tuple(8, 0, startButton_),
	});
	settings->set_margin(9);
	auto panel = new grid(
	{ },
	{ grid_length(2, mnfx::grid_unit_type::star), 320 },
	{
		make_tuple(0, 0, evrPanel_),
		make_tuple(0, 1, settings)
	});
	set_child(panel);
}

LRESULT DebugWindow::window_procedure(mnfx::window_message message, WPARAM wparam, LPARAM lparam, bool& handled) noexcept
{
	using namespace mnfx;
	if (message == window_message::device_change)
	{
		ctx_.ChangingDevice();
		handled = true;
	}
	return window::window_procedure(message, wparam, lparam, handled);
}

HRESULT DebugWindow::on_initialize() noexcept
{
	size_t sid = 0;
	ctx_.GraphStarted().add(bind(&DebugWindow::GraphStartCallback, this, placeholders::_1, placeholders::_2), sid);
	size_t vid = 0;
	ctx_.VideoDeviceChanged().add(bind(&DebugWindow::VideoDeviceChangeCallback, this, placeholders::_1, placeholders::_2), vid);
	size_t vid2 = 0;
	ctx_.VideoMediaTypeChanged().add(bind(&DebugWindow::VideoMediaTypeChangeCallback, this, placeholders::_1, placeholders::_2), vid2);
	size_t aid = 0;
	ctx_.AudioDeviceChanged().add(bind(&DebugWindow::AudioDeviceChangeCallback, this, placeholders::_1, placeholders::_2), aid);
	size_t aid2 = 0;
	ctx_.AudioMediaTypeChanged().add(bind(&DebugWindow::AudioMediaTypeChangeCallback, this, placeholders::_1, placeholders::_2), aid2);

	ctx_.Initialize();
	return window::on_initialize();
}

HRESULT DebugWindow::VideoMediaTypeUpdateCallback(::mnfx::combo_box_base const& sender, value_change_event_args<wstring> args) noexcept
{
	auto st = Split(args.new_value);
	if (st.size() != 4 && st.size() != 5) return E_FAIL;

	auto resText = st[0].to_string();
	auto resBuf = Split(resText, L'x');
	if (resBuf.size() != 2) return E_FAIL;
	auto width = resBuf[0].to_integer<int16_t>();
	auto height = resBuf[1].to_integer<int16_t>();
	ctx_.SetHeightAndWidth(make_pair(height, width));

	auto bitText = st[1].to_string();
	auto bitBuf = Split(bitText, L' ');
	if (bitBuf.size() < 2) return E_FAIL;
	auto bit = bitBuf[0].to_integer<int8_t>();
	ctx_.SetBit(bit);

	auto fpsText = st[2].to_string();
	auto fpsBuf = Split(fpsText, L' ');
	if (fpsBuf.size() < 2) return E_FAIL;
	auto fps = fpsBuf[0].to_integer<int16_t>();
	ctx_.SetFramerate(fps);

	auto fourccText = st[3].to_string();
	auto fourccBuf = Split(fourccText, L' ');
	if (fourccBuf.size() != 1) return E_FAIL;
	auto fourccStr = fourccBuf[0].to_string();
	char fourcc[5] = { 0 };
	size_t count = 0;
	wcstombs_s(&count, fourcc, fourccStr.c_str(), 5);
	ctx_.SetFourCC(fourcc);

	if (st.size() == 5)
	{
		auto aspectText = st[4].to_string();
		auto aspectBuf = Split(aspectText, L':');
		if (aspectBuf.size() != 2) return E_FAIL;
		auto aspectY = aspectBuf[1].to_integer<int16_t>();
		auto aspectX = aspectBuf[0].to_integer<int16_t>();
		ctx_.SetAspect(make_pair(aspectY, aspectX));
	}
	else
	{
		ctx_.SetAspect(make_pair(-1, -1));
	}

	return S_OK;
}

HRESULT DebugWindow::AudioMediaTypeUpdateCallback(::mnfx::combo_box_base const& sender, value_change_event_args<wstring> args) noexcept
{
	auto st = Split(args.new_value);
	if (st.size() != 3) return E_FAIL;

	auto channelText = st[0].to_string();
	auto channelBuf = Split(channelText, L' ');
	if (channelBuf.size() != 3) return E_FAIL;
	auto channel = channelBuf[1].to_integer<int16_t>();
	ctx_.SetChannel(channel);

	auto sampleRateText = st[1].to_string();
	auto sampleRateBuf = Split(sampleRateText, L' ');
	if (sampleRateBuf.size() != 2) return E_FAIL;
	auto sampleRate = sampleRateBuf[0].to_integer<int32_t>();
	ctx_.SetSampleRate(sampleRate);

	auto bitText = st[2].to_string();
	auto bitBuf = Split(bitText, L' ');
	if (bitBuf.size() != 2) return E_FAIL;
	auto bit = bitBuf[0].to_integer<int16_t>();
	ctx_.SetAudioBit(bit);
	return S_OK;
}

HRESULT DebugWindow::StartClickCallback(button const& sender, event_args args) noexcept
{
	try
	{
		ctx_.Start();
	}
	catch (AppException e)
	{
		MessageBoxW(hwnd(), e.Message().c_str(), L"Error", MB_OK);
	}
	return S_OK;
}

HRESULT DebugWindow::GraphStartCallback(AppContext const& sender, event_args args) noexcept
{
	evrPanel_->SetVideoDisplayControl(ctx_.GetVideoDisplayControl());
	return S_OK;
}

HRESULT DebugWindow::VideoDeviceChangeCallback(AppContext const& sender, event_args args) noexcept
{
	auto& vec = *reinterpret_cast<observable_vector_impl<wstring>*>(&videoDevicesBox_->items());
	vec.clear();

	auto& devices = ctx_.VideoDevices();
	for (auto&& device : devices)
	{
		vec.push_back(device);
	}
	return S_OK;
}

HRESULT DebugWindow::VideoMediaTypeChangeCallback(AppContext const& sender, event_args args) noexcept
{
	auto& vec = *reinterpret_cast<observable_vector_impl<wstring>*>(&videoMediaTypesBox_->items());
	vec.clear();

	auto& mts = ctx_.VideoMediaTypes();
	for (auto&& mt : mts)
	{
		::std::wstringstream stream;
		if (mt.IsVideo())
		{
			if (mt.IsVideoInfoHeader())
			{
				auto& vi = mt.AsVideoInfoHeader();
				stream
					<< vi.bmiHeader.biWidth << L'x' << vi.bmiHeader.biHeight << L", "
					<< vi.bmiHeader.biBitCount << L" bit, "
					<< (10000000.0 / static_cast<float>(vi.AvgTimePerFrame)) << L" fps, "
					<< mt.GetFourCC().c_str();
			}
			else if (mt.IsVideoInfoHeader2())
			{
				auto& vi = mt.AsVideoInfoHeader2();
				stream
					<< vi.bmiHeader.biWidth << L'x' << vi.bmiHeader.biHeight << L", "
					<< vi.bmiHeader.biBitCount << L" bit, "
					<< (10000000.0 / static_cast<float>(vi.AvgTimePerFrame)) << L" fps, "
					<< mt.GetFourCC().c_str() << L", "
					<< vi.dwPictAspectRatioX << L':' << vi.dwPictAspectRatioY;
			}
		}
		vec.push_back(stream.str().c_str());
	}
	return S_OK;
}

HRESULT DebugWindow::AudioDeviceChangeCallback(AppContext const& sender, event_args args) noexcept
{
	auto& vec = *reinterpret_cast<observable_vector_impl<wstring>*>(&audioDevicesBox_->items());
	vec.clear();

	auto& devices = ctx_.AudioDevices();
	for (auto&& device : devices)
	{
		vec.push_back(device);
	}
	return S_OK;
}

HRESULT DebugWindow::AudioMediaTypeChangeCallback(AppContext const& sender, event_args args) noexcept
{
	auto& vec = *reinterpret_cast<observable_vector_impl<wstring>*>(&audioMediaTypesBox_->items());
	vec.clear();

	auto& mts = ctx_.AudioMediaTypes();
	for (auto&& mt : mts)
	{
		::std::wstringstream stream;
		if (mt.IsAudio())
		{
			if (mt.mt_->subtype == MEDIASUBTYPE_PCM) stream << L"PCM ";

			if (mt.IsWaveFormatEx())
			{
				auto& wf = mt.AsWaveFormatEx();
				stream << wf.nChannels << L" ch, " << wf.nSamplesPerSec << L" Hz, " << wf.wBitsPerSample << " bit";
			}
		}
		vec.push_back(stream.str().c_str());
	}
	return S_OK;
}