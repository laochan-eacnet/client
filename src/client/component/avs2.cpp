#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "component/filesystem.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <game/game.hpp>
#include <launcher/launcher.hpp>

namespace logger
{
	int __stdcall msgbox_hook(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
	{
		auto msg = utils::string::shiftjis_to_wide(lpText);
		auto caption = utils::string::shiftjis_to_wide(lpCaption);

		return MessageBoxW(hWnd, msg.data(), caption.data(), uType);
	}

	WORD log_level_color(int level)
	{
		switch (level)
		{
		case 1:
			return FOREGROUND_RED;
		case 2:
			return FOREGROUND_RED | FOREGROUND_GREEN;
		case 3:
			return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
		case 4:
			return FOREGROUND_INTENSITY;
		default:
			break;
		}

		return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
	}

	int avs_boot_hook(game::node_ptr config, void* heap, size_t heap_size, void*, void*, HANDLE)
	{
		return game::avs_boot(config, heap, heap_size, nullptr, /*output_callback*/ nullptr, nullptr);
	}

	int read_config(uint32_t, char* data, uint32_t size)
	{
		static int n = 0;
		static FILE* f = nullptr;

		if (!f)
		{
			fopen_s(&f, "config.kbin", "rb");

			if (!f) return -2;
		}

		if (n++ == 1)
			fseek(f, 0, SEEK_SET);

		auto result = fread(data, 1, size, f);

		if (result < size)
			fclose(f);

		return static_cast<int>(result);
	}

	class component final : public component_interface
	{
	public:
		void post_start()
		{
			utils::hook::nop(0x1400C685A, 6);
			utils::hook::call(0x1400C685A, avs_boot_hook);

#if DEBUG
			// utils::hook::jump(0x1400C6A30, read_config);
			utils::hook::iat(utils::nt::library{}, "user32.dll", "MessageBoxA", msgbox_hook);

			SetConsoleCP(932);
#endif
		}
	};
}

REGISTER_COMPONENT(logger::component)
