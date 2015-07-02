#include "pch.hpp"
#include "DebugWindow.hpp"

#include "layouts.hpp"
#include "button.hpp"
#include "EvrPanel.hpp"

using namespace std;
using namespace mnfx;

DebugWindow::DebugWindow(::Microsoft::WRL::ComPtr<IMFVideoDisplayControl> videoDisplayControl)
	: window()
{
	set_text(L"Monotor Debug Window");
	set_left(48);
	set_height(360);
	set_width(960);

	auto evr = new EvrPanel(videoDisplayControl);
	auto panel = new grid(
	{ },
	{ grid_length(2, mnfx::grid_unit_type::star), grid_length() },
	{
		make_tuple(0, 0, evr),
		make_tuple(0, 1, new button(L"test"))
	});
	set_child(panel);
}

LRESULT DebugWindow::window_procedure(mnfx::window_message message, WPARAM wparam, LPARAM lparam, bool& handled) noexcept
{
	using namespace mnfx;
	if (message == window_message::command)
	{
		const HWND& hwnd = reinterpret_cast<HWND>(lparam);

	}
	return window::window_procedure(message, wparam, lparam, handled);
}

HRESULT DebugWindow::on_create() noexcept
{

	return S_OK;
}