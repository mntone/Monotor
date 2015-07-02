#pragma once
#include "window.hpp"

class DebugWindow final
	: public mnfx::window
{
public:
	DebugWindow(::Microsoft::WRL::ComPtr<IMFVideoDisplayControl> videoDisplayControl);

protected:
	virtual LRESULT window_procedure(mnfx::window_message message, WPARAM wparam, LPARAM lparam, bool& handled) noexcept;
	HRESULT on_create() noexcept;
};