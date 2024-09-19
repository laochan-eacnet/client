#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "d3d9_proxy/interface_ex.hpp"

namespace iidx::custom_resolution
{
	int mode()
	{
		static auto mode = std::stoi(game::environment::get_param("IIDX_DISPLAY_MODE"));
		return mode;
	}

	int width()
	{
		static auto w = mode() == 1 ?
			GetSystemMetrics(SM_CXSCREEN) : std::stoi(game::environment::get_param("IIDX_RESOLTION_W"));
		if (w == 0)//不知道为什么本机分辨率的时候参数会为0 总之在这处理一下
		{
			return GetSystemMetrics(SM_CXSCREEN);
		}
		return 1920;
	}

	int height()
	{
		static auto h = mode() == 1 ?
			GetSystemMetrics(SM_CYSCREEN) : std::stoi(game::environment::get_param("IIDX_RESOLTION_H"));
		if (h == 0)
		{
			return GetSystemMetrics(SM_CYSCREEN);
		}

		return 1200;
	}

	namespace
	{
		HRESULT WINAPI create_d3d9ex(UINT SDKVersion, IDirect3D9Ex** ppD3D9Ex)
		{
			IDirect3D9Ex* d3d9ex = nullptr;

			auto hr = Direct3DCreate9Ex(SDKVersion, &d3d9ex);

			if (SUCCEEDED(hr))
			{
				*ppD3D9Ex = new d3d9ex_proxy(d3d9ex);
			}
			else
			{
				*ppD3D9Ex = nullptr;
			}

			return hr;
		}

		BOOL __stdcall set_rect(LPRECT lprc, int xLeft, int yTop, int xRight, int yBottom)
		{
			lprc->left = 0;
			lprc->top = 0;
			lprc->right = width();
			lprc->bottom = height();

			return TRUE;
		}

		HWND WINAPI create_window_ex_a(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
		{
			if (mode() == 2)
			{
				dwStyle = WS_SYSMENU | WS_CAPTION | WS_VISIBLE;
				RECT rc;
				SetRect(&rc, 0, 0, width(), height());
				AdjustWindowRect(&rc, WS_VISIBLE | WS_CAPTION, false);

				nWidth = rc.right - rc.left;
				nHeight = rc.bottom - rc.top;
			}
			else
			{
				nWidth = width();
				nHeight = height();
			}

			return CreateWindowExA(dwExStyle ^ WS_EX_TOPMOST, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		}

		BOOL WINAPI set_window_pos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
		{
			if (mode() == 2)
			{
				RECT rc;
				SetRect(&rc, 0, 0, width(), height());
				AdjustWindowRect(&rc, WS_VISIBLE | WS_CAPTION, false);

				cx = rc.right - rc.left;
				cy = rc.bottom - rc.top;
			}
			else
			{
				cx = width();
				cy = height();
			}

			return SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
		}
	}

	class component final : public component_interface
	{
	public:

		void* load_import(const std::string& library, const std::string& function) override
		{
			if (function == "Direct3DCreate9Ex")
			{
				return create_d3d9ex;
			}
			else if (function == "SetRect")
			{
				return set_rect;
			}
			else if (function == "CreateWindowExA")
			{
				return create_window_ex_a;
			}
			else if (function == "SetWindowPos")
			{
				return set_window_pos;
			}

			return nullptr;
		}
	};
}

REGISTER_COMPONENT(iidx::custom_resolution::component, launcher::game::iidx)
