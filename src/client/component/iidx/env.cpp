#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include <game/game.hpp>
#include <launcher/launcher.hpp>

namespace env
{
	const char* get_cmdline()
	{
		static auto cmdline = ([]
			{
				auto args = "bm2dx.exe -t "s;
				args += game::environment::get_param("LAOCHAN_TOKEN");

				if (game::environment::get_param("IIDX_SOUND_DEVICE") == "1")
					args += " --asio ";

				auto display_mode = game::environment::get_param("IIDX_DISPLAY_MODE");

				if (display_mode == "1")
					args += " --miniW ";
				else if (display_mode == "2")
					args += " -w ";
				else if (display_mode == "3")
					args += " --fitW ";

				printf("Starting game with cmdline: %s\n", args.data());
				return args;
			}
		)();

		return cmdline.data();
	}

	int WINAPI message_box_a(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
	{
		auto text = utils::string::shiftjis_to_wide(lpText);
		auto caption = utils::string::shiftjis_to_wide(lpCaption);

		return MessageBoxW(hWnd, text.data(), caption.data(), uType);
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
			if (version != "P2D:J:B:A:2024052200")
			{
				throw std::runtime_error(utils::string::va("Unsupported version %s\n", version.data()));
			}
		}

		void* load_import(const std::string& library, const std::string& function) override
		{
			if (function == "GetCommandLineA")
			{
				return get_cmdline;
			}
			else if (function == "MessageBoxA")
			{
				return message_box_a;
			}

			return nullptr;
		}
	};
}

REGISTER_COMPONENT(env::component, launcher::game::iidx)
