#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <game/game.hpp>
#include <launcher/launcher.hpp>
#include <DbgHelp.h>

namespace sdvx::patches
{
	bool is_signature_valid()
	{
		return true;
	}

	BOOL is_debugger_present()
	{
		return false;
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			utils::nt::library libeacnet{ "libeacnet.dll" };

			// disable signature check
			utils::hook::jump(libeacnet.get_ptr() + 0xF9F0, is_signature_valid, true);
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

REGISTER_COMPONENT(sdvx::patches::component, launcher::game::sdvx)
