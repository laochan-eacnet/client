#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <game/game.hpp>
#include <launcher/launcher.hpp>

char asio_name[0x2048];

namespace patches
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
			// disable 120hz limit
			// utils::hook::nop(0x1401F5F54, 2);
			// change server url
			utils::hook::jump(0x1402EBD00, get_service_url);
			printf("I:launcher: using bootstrap url: %s\n", launcher::get_service_address.data());

			// override asio device name
			if (launcher::asio_device_name.size() > 0x2047) 
				throw std::exception("ASIO Device Name is too long!");
			
			std::memcpy(asio_name, launcher::asio_device_name.data(), launcher::asio_device_name.size());

			utils::hook::inject(0x140246F74, asio_name);
			printf("I:launcher: using asio device: %s\n", launcher::asio_device_name.data());
		}
	};
}

REGISTER_COMPONENT(patches::component)
