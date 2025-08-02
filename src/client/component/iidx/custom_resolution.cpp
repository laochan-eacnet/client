#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "d3d9_proxy/interface_ex.hpp"

#include "custom_resolution.hpp"

namespace iidx::custom_resolution
{
	IIDX_DISPLAY_MODE_ mode()
	{
		static auto mode = std::stoi(game::environment::get_param("IIDX_DISPLAY_MODE"));
		return static_cast<IIDX_DISPLAY_MODE_>(mode);
	}

	int width()
	{
		static auto w = mode() == IIDX_DISPLAY_MODE_FULLSCREEN_WINDOWED ?
			GetSystemMetrics(SM_CXSCREEN) : std::stoi(game::environment::get_param("IIDX_RESOLTION_W"));

		return w ? w : GetSystemMetrics(SM_CXSCREEN);
	}

	int height()
	{
		static auto h = mode() == IIDX_DISPLAY_MODE_FULLSCREEN_WINDOWED ?
			GetSystemMetrics(SM_CYSCREEN) : std::stoi(game::environment::get_param("IIDX_RESOLTION_H"));

		return h ? h : GetSystemMetrics(SM_CYSCREEN);
	}

	IIDX_GRAPHICS_API_ graphics_api()
	{
		static auto api = std::stoi(game::environment::get_param("IIDX_GRAPHICS_API"));
		return static_cast<IIDX_GRAPHICS_API_>(api);
	}

	namespace
	{
		HRESULT WINAPI create_d3d9ex(UINT SDKVersion, IDirect3D9Ex** ppD3D9Ex)
		{
			IDirect3D9Ex* d3d9ex = nullptr;
			auto hr = E_FAIL;
			const auto api = graphics_api();

			switch (api)
			{
				case IIDX_GRAPHICS_API_DX9:
				{
					hr = Direct3DCreate9Ex(SDKVersion, &d3d9ex);
					break;
				}
				case IIDX_GRAPHICS_API_DX9_ON_12:
				{
					void* device = nullptr;
					hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_9_3, __uuidof(ID3D12Device), &device);
					if (!SUCCEEDED(hr))
						break;

					D3D9ON12_ARGS arg
					{
						.Enable9On12 = true,
						.pD3D12Device = reinterpret_cast<IUnknown*>(device),
						.NumQueues = 0,
					};

					hr = Direct3DCreate9On12Ex(SDKVersion, &arg, 1, &d3d9ex);
					break;
				}
				case IIDX_GRAPHICS_API_DXVK:
				{
					auto dxvk = utils::nt::library{ "dxvk.dll" };
					if (!dxvk)
						break;

					hr = dxvk.invoke_pascal<HRESULT>("Direct3DCreate9Ex", SDKVersion, &d3d9ex);
					break;
				}
			}

			if (FAILED(hr))
			{
				printf("W: Failed to initialize graphics api with mode %d, falling back to d3d9, hr = 0x%x.\n", api, hr);
				hr = Direct3DCreate9Ex(SDKVersion, &d3d9ex);
			}

			if (SUCCEEDED(hr))
			{
				if (mode() == IIDX_DISPLAY_MODE_FULLSCREEN)
					DwmEnableMMCSS(TRUE);

				*ppD3D9Ex = new d3d9ex_proxy(d3d9ex);
			}
			else
			{
				printf("E: Failed to initialize graphics api with hr = 0x%x.\n", hr);
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
			if (mode() == IIDX_DISPLAY_MODE_WINDOWED)
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
			if (mode() == IIDX_DISPLAY_MODE_WINDOWED)
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

		HRESULT WINAPI co_create_instance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID* ppv) 
		{
			if (IsEqualCLSID(rclsid, __uuidof(MMDeviceEnumerator)) && IsEqualIID(riid, __uuidof(IMMDeviceEnumerator)))
			{
				IMMDeviceEnumerator* enumerator = nullptr;
			}
			else
			{
				return CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
			}
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
			else if(function == "CoCreateInstance")
			{
				return co_create_instance;
			}

			return nullptr;
		}
	};
}

REGISTER_COMPONENT(iidx::custom_resolution::component, launcher::game::iidx)
