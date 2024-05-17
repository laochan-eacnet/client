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

	bool __stdcall is_unlocked(int item_type, int itemId, int param)
	{
		printf("check item %d\n", itemId);
		return true;
	}

	game::symbol<unsigned int __cdecl(void* __formal, void* buffer, unsigned int size, char const* unk2)> get_key_salt{ 0x5AB590 };
	game::symbol<unsigned int __cdecl(void* __formal, void* buffer, unsigned int size, char const* unk2)> get_path_salt{ 0x5AB4F0 };

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

			utils::hook::jump(0x4A3D60, is_unlocked, true);
		}

#ifdef DUMP_DATA
		void post_load() override
		{
			char key_salt[40] = { 0 };
			get_path_salt(0, key_salt, 40, nullptr);

			printf("M:extdrmfs: key salt: %s\n", key_salt);

			char path_salt[10] = { 0 };
			get_key_salt(0, path_salt, 10, nullptr);

			printf("M:extdrmfs: path salt: %s\n", path_salt);
		}
#endif
	};
}

REGISTER_COMPONENT(patches::component)
