#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include <game/game.hpp>
#include <launcher/launcher.hpp>

#define IIDX_TARGET_VERSION "P2D:J:B:A:2024052200"

namespace env
{
	const char* get_cmdline()
	{
		static auto cmdline = ([]
			{
				auto args = "bm2dx.exe --ex -t "s;
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

	utils::hook::detour init_thread_footer;
	__int64 __fastcall init_thread_footer_hook(DWORD* a1)
	{
		// set language
		static auto language = ([]
			{
				return std::stoi(game::environment::get_param("IIDX_LANGUAGE"));
			}
		)();
		
		if (language)
		{
			utils::hook::set<uint32_t>(0x1405F0CD0, 1);
			utils::hook::set<uint32_t>(0x1405F0CD4, language);
		}

		return init_thread_footer.invoke<__int64>(a1);
	}

	utils::hook::detour load_ifs;
	char __fastcall load_ifs_hook(char* a1, __int64 a2, int a3, const char* a4, unsigned int a5, char a6)
	{
		printf("load ifs %s\n", a4);



		return load_ifs.invoke<char>(a1, a2, a3, a4, a5, a6);
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
			if (version != IIDX_TARGET_VERSION)
			{
				throw std::runtime_error(utils::string::va("Unsupported version %s\nSupported version is " IIDX_TARGET_VERSION ".", version.data()));
			}

			load_ifs.create(0x1402240E0, load_ifs_hook);
			init_thread_footer.create(0x14035EFA4, init_thread_footer_hook);
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
