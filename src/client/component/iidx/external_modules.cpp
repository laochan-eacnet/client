#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <game/game.hpp>

namespace iidx::external_modules
{
	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			if (game::environment::get_param("IIDX_USE_GSM") != "1")
				return;

			LoadLibraryA("2dx-gsm.dll");
		}
	};
}

REGISTER_COMPONENT(iidx::external_modules::component, launcher::game::iidx)
