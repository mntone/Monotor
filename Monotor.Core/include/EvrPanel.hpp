#pragma once
#include "static_panel.hpp"

class EvrPanel final
	: public mnfx::static_panel
{
public:
	EvrPanel(::Microsoft::WRL::ComPtr<IMFVideoDisplayControl> videoDisplayControl);

protected:
	virtual HRESULT on_initialize() noexcept final;
	virtual HRESULT on_resize(::mnfx::size size) noexcept final;
	virtual HRESULT on_rearrange(::mnfx::rect rect) noexcept final;

private:
	::Microsoft::WRL::ComPtr<IMFVideoDisplayControl> videoDisplayControl_;
};