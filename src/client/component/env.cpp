#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <game/game.hpp>
#include <launcher/launcher.hpp>

namespace env
{
	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

			std::string args{ "-t " };
			args += launcher::token;
			args += " ";

			if (launcher::snd_mode == launcher::sound_mode::asio) {
				args += "--asio ";
			}

			switch (launcher::disp_mode) {
				case launcher::display_mode::windowed_1080p:
					args += "--fitW ";
					break;
				case launcher::display_mode::windowed_900p:
					args += "-w ";
					break;
				case launcher::display_mode::windowed_720p:
					args += "--miniW ";
					break;
			}

			printf("I:launcher: preinit game with arg: %s\n", args.data());
			game::game_preinit(args.data());
		}
	};
}

REGISTER_COMPONENT(env::component)
