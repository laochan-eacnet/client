#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <game/game.hpp>
#include <launcher/launcher.hpp>
#include <DbgHelp.h>

char asio_name[0x2048];

namespace patches
{
	bool __stdcall is_signature_valid(int, int, int)
	{
		return true;
	}

	bool is_debugger_present()
	{
		return false;
	}

	const char* get_command_line()
	{
		return launcher::get_args();
	}

	const char* __stdcall get_service_url(bool, char*)
	{
		return launcher::get_service_address.data();
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			utils::nt::library kernel32{ "kernel32.dll" };
			utils::nt::library libeacnet{ "libeacnet.dll" };

			// disable debugger trap
			utils::hook::jump(kernel32.get_proc<void*>("IsDebuggerPresent"), is_debugger_present, true);

			utils::hook::jump(libeacnet.get_ptr() + 0xD9A0, is_signature_valid, true);
			utils::hook::jump(libeacnet.get_ptr() + 0xF680, get_service_url, true);

			utils::hook::nop(0x5AC0CE, 6);
			utils::hook::call(0x5AC0CE, get_command_line);
		}
	};
}

REGISTER_COMPONENT(patches::component)
