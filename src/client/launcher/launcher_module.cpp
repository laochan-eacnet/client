#include <std_include.hpp>
#include "launcher_module.hpp"
#include <dwmapi.h>
#include <gdiplus.h>
#include <winuser.h>

#include "utils/nt.hpp"

extern "C"
{
	typedef enum
	{
		WCA_ACCENT_POLICY = 19,
	} WINDOWCOMPOSITIONATTRIB;

	typedef struct tagWINDOWCOMPOSITIONATTRIBDATA
	{
		WINDOWCOMPOSITIONATTRIB Attrib;
		void* pvData;
		UINT cbData;
	} WINDOWCOMPOSITIONATTRIBDATA;

	typedef enum _ACCENT_STATE
	{
		ACCENT_DISABLED,
		ACCENT_ENABLE_GRADIENT,
		ACCENT_ENABLE_TRANSPARENTGRADIENT,
		ACCENT_ENABLE_BLURBEHIND,
		ACCENT_ENABLE_ACRYLICBLURBEHIND,
		ACCENT_ENABLE_HOSTBACKDROP,
		ACCENT_INVALID_STATE,
	} ACCENT_STATE;

	typedef struct _ACCENT_POLICY
	{
		ACCENT_STATE AccentState;
		DWORD AccentFlags;
		DWORD GradientColor;
		DWORD AnimationId;
	} ACCENT_POLICY;
}

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

	ACCENT_POLICY ap
	{
		ACCENT_ENABLE_BLURBEHIND,
		0,
		0,
		0
	};

	WINDOWCOMPOSITIONATTRIBDATA ca
	{
		WCA_ACCENT_POLICY,
		&ap,
		sizeof(ap),
	};

	utils::nt::library user32{ "user32.dll" };
	user32.invoke_pascal<BOOL>("SetWindowCompositionAttribute", hwnd, &ca);

	SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

	scale_ = GetDpiForWindow(hwnd) * 1.0f / USER_DEFAULT_SCREEN_DPI;

	auto wv2 = webview->web_view.Get();
	ICoreWebView2Settings* settings;

	if (wv2->get_Settings(&settings) == S_OK)
	{
		settings->put_AreDefaultContextMenusEnabled(false);
		settings->put_IsStatusBarEnabled(false);
	}

	auto wv2c = webview->controller.Get();
	ICoreWebView2Controller2* controller2 = nullptr;
	if (wv2c->QueryInterface(__uuidof(ICoreWebView2Controller2), reinterpret_cast<void**>(&controller2)) == S_OK)
	{
		controller2->put_DefaultBackgroundColor({ 0,0,0,0 });
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
