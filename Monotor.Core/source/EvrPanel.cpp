#include "pch.hpp"
#include "EvrPanel.hpp"

#include "window.hpp"

using namespace mnfx;
using namespace Microsoft::WRL;

EvrPanel::EvrPanel()
{
	set_style(style() | mnfx::window_style::static_owner_draw);
}

HRESULT EvrPanel::on_initialize() noexcept
{
	if (!videoDisplayControl_) return S_OK;

	return videoDisplayControl_->SetVideoWindow(hwnd());
}

HRESULT EvrPanel::on_resize(mnfx::size size) noexcept
{
	HRESULT hr = static_panel::on_resize(size);
	if (FAILED(hr)) return hr;
	if (!videoDisplayControl_) return S_OK;

	auto& scale_factor = root().scale_factor();
	MFVideoNormalizedRect mvnr = { 0, 0, 1, 1 };
	RECT rect = { 0, 0, static_cast<int32_t>(scale_factor.scale_x(size.width)), static_cast<int32_t>(scale_factor.scale_y(size.height)) };
	return videoDisplayControl_->SetVideoPosition(&mvnr, &rect);
}

HRESULT EvrPanel::on_rearrange(mnfx::rect rect) noexcept
{
	HRESULT hr = static_panel::on_rearrange(rect);
	if (FAILED(hr)) return hr;
	if (!videoDisplayControl_) return S_OK;

	auto& scale_factor = root().scale_factor();
	MFVideoNormalizedRect mvnr = { 0, 0, 1, 1 };
	RECT rect2 = { 0, 0, static_cast<int32_t>(scale_factor.scale_x(rect.width)), static_cast<int32_t>(scale_factor.scale_y(rect.height)) };
	return videoDisplayControl_->SetVideoPosition(&mvnr, &rect2);
}

void EvrPanel::SetVideoDisplayControl(::Microsoft::WRL::ComPtr<IMFVideoDisplayControl> videoDisplayControl)
{
	videoDisplayControl.CopyTo(&videoDisplayControl_);
	if (videoDisplayControl_)
	{
		videoDisplayControl_->SetVideoWindow(hwnd());

		auto& scale_factor = root().scale_factor();
		MFVideoNormalizedRect mvnr = { 0, 0, 1, 1 };
		RECT rect = { 0, 0, 0, 0 };
		GetWindowRect(hwnd(), &rect);
		rect.top = 0;
		rect.left = 0;
		videoDisplayControl_->SetVideoPosition(&mvnr, &rect);
	}
}