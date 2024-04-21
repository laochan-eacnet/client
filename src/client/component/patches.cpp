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
	bool is_signature_valid()
	{
		return true;
	}

	bool is_debugger_present()
	{
		return false;
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{

		}
	};
}

REGISTER_COMPONENT(patches::component)
