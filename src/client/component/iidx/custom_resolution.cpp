#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "d3d9_proxy/interface_ex.hpp"

namespace iidx::custom_resolution
{
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
			lprc->right = 2560;
			lprc->bottom = 1440;

			return TRUE;
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

			return nullptr;
		}
	};
}

REGISTER_COMPONENT(iidx::custom_resolution::component, launcher::game::iidx)
