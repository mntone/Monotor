#include "pch.hpp"
#include "EvrPanel.hpp"

#include "window.hpp"

using namespace mnfx;
using namespace Microsoft::WRL;

EvrPanel::EvrPanel(ComPtr<IMFVideoDisplayControl> videoDisplayControl)
	: videoDisplayControl_(videoDisplayControl)
{
	set_style(style() | mnfx::window_style::static_owner_draw);
}

HRESULT EvrPanel::on_initialize() noexcept
{
	return videoDisplayControl_->SetVideoWindow(hwnd());
}

HRESULT EvrPanel::on_resize(mnfx::size size) noexcept
{
	HRESULT hr = static_panel::on_resize(size);
	if (FAILED(hr)) return hr;

	auto scale_factor = root().scale_factor();
	MFVideoNormalizedRect mvnr = { 0, 0, 1, 1 };
	RECT rect = { 0, 0, static_cast<int32_t>(scale_factor.scale_x(size.width)), static_cast<int32_t>(scale_factor.scale_y(size.height)) };
	return videoDisplayControl_->SetVideoPosition(&mvnr, &rect);
}

HRESULT EvrPanel::on_rearrange(mnfx::rect rect) noexcept
{
	HRESULT hr = static_panel::on_rearrange(rect);
	if (FAILED(hr)) return hr;

	auto scale_factor = root().scale_factor();
	MFVideoNormalizedRect mvnr = { 0, 0, 1, 1 };
	RECT rect2 = { 0, 0, static_cast<int32_t>(scale_factor.scale_x(rect.width)), static_cast<int32_t>(scale_factor.scale_y(rect.height)) };
	return videoDisplayControl_->SetVideoPosition(&mvnr, &rect2);
}