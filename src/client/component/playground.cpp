#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/string.hpp>
#include <utils/hook.hpp>
#include <game/game.hpp>

namespace playground
{
	game::symbol<uint64_t(uint64_t a1, char* buf, size_t size)> get_key_salt{0x1400B9E80};
	game::symbol<uint64_t(uint64_t a1, char* buf)> get_path_salt{ 0x1400BA0E0 };

	uint64_t avs_callback_a_h(uint64_t a1, uint64_t a2, uint64_t a3)
	{
		return 0;
	}

	uint64_t avs_callback_b_h(uint64_t a1, uint64_t a2)
	{
		return 0;
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			// utils::hook::jump(0x1400B9E80, avs_callback_a_h);
			// utils::hook::jump(0x1400BA0E0, avs_callback_b_h);
		}

		void post_load() override
		{
			char key_salt[40] = { 0 };
			get_key_salt(0, key_salt, 40);
			
			printf("M:extdrmfs: key salt: %s\n", key_salt);

			char path_salt[10] = { 0 };
			get_path_salt(0, path_salt);

			printf("M:extdrmfs: path salt: %s\n", path_salt);
		}
	};
}

REGISTER_COMPONENT(playground::component)
