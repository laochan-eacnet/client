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
	
	bool is_signature_valid()
	{
		return true;
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			// disable signature check
			utils::hook::jump(0x140312A00, is_signature_valid);

			// disable music list checksum check
			utils::hook::nop(0x140301682, 2);
			utils::hook::nop(0x14030168E, 2);
	
			// change server url
			utils::hook::jump(0x1402F8750, get_service_url);
			printf("I:launcher: using bootstrap url: %s\n", launcher::get_service_address.data());

			// override asio device name
			if (launcher::asio_device_name.size() > 0x2047) 
				throw std::exception("ASIO Device Name is too long!");
			
			std::memcpy(asio_name, launcher::asio_device_name.data(), launcher::asio_device_name.size());

			utils::hook::inject(0x1402539C4, asio_name);
			printf("I:launcher: using asio device: %s\n", launcher::asio_device_name.data());

			// enable retry logic for asio
			utils::hook::nop(0x1401DC737, 6);
			init_superstep_sound_hook.create(0x1402536E0, init_superstep_sound_stub);
		}
	};
}

REGISTER_COMPONENT(patches::component)
