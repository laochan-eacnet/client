#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <game/game.hpp>
#include <launcher/launcher.hpp>
#include <DbgHelp.h>

namespace ddr::patches
{
	bool is_signature_valid()
	{
		return true;
	}

	BOOL is_debugger_present()
	{
		return false;
	}

	// force exit anti debug thread
	void check_remote_debugger_preset(HANDLE, PBOOL preset)
	{
		ExitThread(0);
	}

	void* get_proc_address(HMODULE module, const char* name)
	{
		if (name == "IsDebuggerPresent"s)
			return is_debugger_present;

		if (name == "CheckRemoteDebuggerPresent"s)
			return check_remote_debugger_preset;

		return GetProcAddress(module, name);
	}

	const char* __fastcall get_service_url()
	{
		static auto service_url = ([]
		{
			return game::environment::get_param("LAOCHAN_SERVER_URL");
		})();

		return service_url.data();
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			utils::nt::library libeacnet{ "libeacnet.dll" };

			// change service url
			auto get_service_url_loc = libeacnet.match_sig("0F B6 93 9B 00 00 00");
			if (!get_service_url_loc)
				throw std::runtime_error{ "Failed to hook service url." };

			auto get_servicel_url_func = utils::hook::extract<uint8_t*>(get_service_url_loc + 8);
			utils::hook::jump(get_servicel_url_func, get_service_url, true);

			// disable signature check
			auto verify_signature_loc = libeacnet.match_sig("FF 90 80 00 00 00 84 C0");
			if (!verify_signature_loc)
				throw std::runtime_error{ "Failed to bypass signature check." };

			// mov al, 1
			utils::hook::set<uint16_t>(verify_signature_loc, 0x01B0);
			utils::hook::nop(verify_signature_loc + 2, 4);
		}

		void* load_import(const std::string& library, const std::string& function) override
		{
			// disable debugger trap
			if (function == "IsDebuggerPresent")
			{
				return is_debugger_present;
			}
			else if (function == "GetProcAddress")
			{
				return get_proc_address;
			}

			return nullptr;
		}
	};
}

REGISTER_COMPONENT(ddr::patches::component, launcher::game::ddr)
