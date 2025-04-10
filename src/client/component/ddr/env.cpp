#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include <game/game.hpp>
#include <launcher/launcher.hpp>

namespace ddr::env
{
	const char* get_cmdline()
	{
		static auto cmdline = ([]
			{
				auto args = "ddr-konaste.exe -t "s;
				args += game::environment::get_param("LAOCHAN_TOKEN");

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

REGISTER_COMPONENT(ddr::env::component, launcher::game::ddr)
