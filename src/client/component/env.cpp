#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <game/game.hpp>
#include <launcher/launcher.hpp>

namespace env
{
	const char* __fastcall get_service_url(void* _this, bool is_dev, char is_kr)
	{
		return launcher::get_service_address.data();
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			// disable signature check
			utils::hook::set<uint8_t>(0x1403050B5, 0xEB);
			utils::hook::jump(0x1402EBD00, get_service_url, true);

			std::string args{ "-t " };
			args += launcher::token;
			args += " ";

			if (launcher::snd_mode == launcher::sound_mode::asio) {
				// override asio device name
				uintptr_t device_name_addr = reinterpret_cast<uintptr_t>(launcher::asio_device_name.data());
				utils::hook::set<uint32_t>(0x140246F74, static_cast<uint32_t>(device_name_addr - 0x140246F78));

				printf("I:launcher: using asio device: %s\n", launcher::asio_device_name.data());
				args += "--asio ";
			}

			switch (launcher::disp_mode) {
				case launcher::display_mode::windowed_1080p:
					args += "--fitW ";
					break;
				case launcher::display_mode::windowed_900p:
					args += "--miniW ";
					break;
				case launcher::display_mode::windowed_720p:
					args += "-w ";
					break;
			}

			printf("I:launcher: preinit game with arg: %s\n", args.data());
			game::game_preinit(args.data());
		}
	};
}

REGISTER_COMPONENT(env::component)
