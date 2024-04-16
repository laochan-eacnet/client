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

	utils::hook::detour init_superstep_sound_hook;
	uint64_t __fastcall init_superstep_sound_stub(void* a1, int /* use_asio */, int a3, int16_t a4, uint32_t a5, int a6, int a7)
	{
		return init_superstep_sound_hook.invoke<uint64_t, void*, int, int, int16_t, uint32_t, int, int>(
			a1, launcher::snd_mode == launcher::sound_mode::asio ? 1 : 0,
			a3, a4, a5, a6, a7
		);
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

			// enable retry logic for asio
			utils::hook::nop(0x1401CFD77, 6);
			init_superstep_sound_hook.create(0x140246C90, init_superstep_sound_stub);
		}
	};
}

REGISTER_COMPONENT(patches::component)
