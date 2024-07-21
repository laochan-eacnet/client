#include <std_include.hpp>
#include "launcher_module.hpp"
#include <dwmapi.h>
#include <gdiplus.h>
#include <winuser.h>

namespace
{
	LONG_PTR wnd_proc_old = NULL;
	LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
	{
		if (msg == WM_NCCALCSIZE && w_param == TRUE)
		{
			SetWindowLong(hwnd, DWLP_MSGRESULT, 0);
			return TRUE;
		}

		return CallWindowProcA(reinterpret_cast<WNDPROC>(wnd_proc_old), hwnd, msg, w_param, l_param);
	}
}

launcher_module::launcher_module(saucer::smartview_core* smartview) : saucer::module(smartview), smartview_(smartview)
{
}

void launcher_module::init(saucer::native::window* window, saucer::native::webview* webview)
{
	window_ = window;
	webview_ = webview;

	// create borderless window with dwm shadow
	auto hwnd = window->hwnd;
	wnd_proc_old = GetWindowLongPtrA(window->hwnd, GWLP_WNDPROC);
	SetWindowLongPtrA(window->hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(wnd_proc));

	SetWindowLongW(hwnd, GWL_STYLE, WS_CAPTION | WS_POPUP);

	MARGINS m{ 0, 0, 0, 1 };
	DwmExtendFrameIntoClientArea(hwnd, &m);

	SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

	scale_ = GetDpiForWindow(hwnd) * 1.0f / USER_DEFAULT_SCREEN_DPI;

	auto wv2 = webview->web_view.Get();
	ICoreWebView2Settings *settings;

	if (wv2->get_Settings(&settings) == S_OK)
	{
		settings->put_AreDefaultContextMenusEnabled(false);
		settings->put_IsStatusBarEnabled(false);
	}
}

void launcher_module::set_dpi_aware_size(int width, int height)
{
	smartview_->set_size(static_cast<int>(width * scale_), static_cast<int>(height * scale_));
}

void launcher_module::send_close()
{
	PostMessage(window_->hwnd, WM_CLOSE, 0, 0);
}
