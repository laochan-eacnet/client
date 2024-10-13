#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include <game/game.hpp>
#include <launcher/launcher.hpp>

#define SDVX_TARGET_VERSION "QCV:J:B:A:2024092500"

namespace sdvx::env
{
	const char* get_cmdline()
	{
		static auto cmdline = ([]
			{
				auto args = "sv6c.exe -t "s;
				args += game::environment::get_param("LAOCHAN_TOKEN");
				args += " -s ";
				args += game::environment::get_param("LAOCHAN_SERVER_URL");

				printf("Starting game with cmdline: %s\n", args.data());
				return args;
			}
		)();

		return cmdline.data();
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		}

		void post_load() override
		{
			auto version = game::environment::get_version();
			if (version != SDVX_TARGET_VERSION)
			{
				throw std::runtime_error(utils::string::va("Unsupported version %s\nSupported version is " SDVX_TARGET_VERSION ".", version.data()));
			}
		}

		void* load_import(const std::string& library, const std::string& function) override
		{
			if (function == "GetCommandLineA")
			{
				return get_cmdline;
			}

			return nullptr;
		}
	};
}

REGISTER_COMPONENT(sdvx::env::component, launcher::game::sdvx)
