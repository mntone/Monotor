#pragma once
#include "window.hpp"
#include "button.hpp"
#include "EvrPanel.hpp"
#include "AppContext.hpp"

namespace mnfx {
class combo_box_base;
class combo_box;
class dropdown_box;
}

namespace Mntone {
namespace Monotor {

class DebugWindow final
	: public mnfx::window
{
public:
	DebugWindow();

protected:
	virtual LRESULT window_procedure(mnfx::window_message message, WPARAM wparam, LPARAM lparam, bool& handled) noexcept;
	virtual HRESULT on_initialize() noexcept;

private:
	HRESULT VideoMediaTypeUpdateCallback(::mnfx::combo_box_base const& sender, ::mnfx::value_change_event_args<::std::wstring> args) noexcept;
	HRESULT AudioMediaTypeUpdateCallback(::mnfx::combo_box_base const& sender, ::mnfx::value_change_event_args<::std::wstring> args) noexcept;
	HRESULT StartClickCallback(::mnfx::button const& sender, ::mnfx::event_args args) noexcept;

	HRESULT GraphStartCallback(AppContext const& sender, ::mnfx::event_args args) noexcept;
	HRESULT VideoDeviceChangeCallback(AppContext const& sender, ::mnfx::event_args args) noexcept;
	HRESULT VideoMediaTypeChangeCallback(AppContext const& sender, ::mnfx::event_args args) noexcept;
	HRESULT AudioDeviceChangeCallback(AppContext const& sender, ::mnfx::event_args args) noexcept;
	HRESULT AudioMediaTypeChangeCallback(AppContext const& sender, ::mnfx::event_args args) noexcept;

private:
	EvrPanel* evrPanel_;
	::mnfx::dropdown_box* videoDevicesBox_;
	::mnfx::combo_box* videoMediaTypesBox_;
	::mnfx::dropdown_box* audioDevicesBox_;
	::mnfx::combo_box* audioMediaTypesBox_;
	::mnfx::button* startButton_;

	::Mntone::Monotor::AppContext ctx_;
};

}
}