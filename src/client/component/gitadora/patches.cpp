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

		if (name == "ExitProcess"s)
			return exit;

		return GetProcAddress(module, name);
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

REGISTER_COMPONENT(gitadora::patches::component, launcher::game::gitadora)
