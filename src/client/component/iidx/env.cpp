#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include <game/game.hpp>
#include <launcher/launcher.hpp>

namespace env
{
	static auto cmdline = "bm2dx.exe "s;

	const char* get_cmdline()
	{
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
			cmdline += "-t sb --miniW";

			/*std::string args{ "-t " };
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
			game::game_preinit(args.data());*/
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
			printf("%s - %s\n", library.data(), function.data());
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
