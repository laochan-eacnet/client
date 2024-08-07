#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include <launcher/launcher.hpp>

namespace gitadora::patches
{
	bool is_signature_valid()
	{
		return true;
	}

	bool is_debugger_present()
	{
		return false;
	}

	const char* __fastcall get_service_url()
	{
		static auto service_url = ([]
			{
				return game::environment::get_param("LAOCHAN_SERVER_URL");
			}
		)();

		return service_url.data();
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			utils::nt::library libeacnet{ "libeacnet.dll" };

			// disable signature check
			utils::hook::jump(libeacnet.get_ptr() + 0xF9E0, is_signature_valid, true);

			// change service url
			utils::hook::jump(libeacnet.get_ptr() + 0x11EC0, get_service_url, true);

			// disable anticheat thread
			utils::hook::nop(0x140003AC2, 5);
		}

		void* load_import(const std::string& library, const std::string& function) override
		{
			// disable debugger trap
			if (function == "IsDebuggerPresent")
			{
				return is_debugger_present;
			}

			return nullptr;
		}
	};
}

REGISTER_COMPONENT(gitadora::patches::component, launcher::game::gitadora)
