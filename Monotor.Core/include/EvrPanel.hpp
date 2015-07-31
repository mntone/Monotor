#pragma once
#include "static_panel.hpp"

class EvrPanel final
	: public mnfx::static_panel
{
public:
	EvrPanel();

protected:
	virtual HRESULT on_initialize() noexcept final;
	virtual HRESULT on_resize(::mnfx::size size) noexcept final;
	virtual HRESULT on_rearrange(::mnfx::rect rect) noexcept final;

public:
	void SetVideoDisplayControl(::Microsoft::WRL::ComPtr<IMFVideoDisplayControl> videoDisplayControl);

private:
	::Microsoft::WRL::ComPtr<IMFVideoDisplayControl> videoDisplayControl_;
};