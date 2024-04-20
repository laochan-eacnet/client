#include <std_include.hpp>
#include "loader/component_loader.hpp"

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

	char log_level_char(int level)
	{
		switch (level)
		{
		case 1:
			return 'F';
		case 2:
			return 'W';
		case 3:
			return 'I';
		case 4:
			return 'M';
		default:
			break;
		}

		return '?';
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

	void avs2_log(void* _this, int level, const char* mod, const char* format, va_list va)
	{
		if (mod == "thread"s || mod == "mutex"s) {
			return;
		}

		static auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(handle, log_level_color(level));

		printf("%c:%s: ", log_level_char(level), mod);
		vprintf_s(format, va);
		putchar('\n');

		SetConsoleTextAttribute(handle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	}

	void on_assert(const wchar_t* Expression, const wchar_t* FunctionName, const wchar_t* FileName, unsigned int LineNo, uintptr_t Reserved)
	{
		wprintf(L"F:logger:Assertion failed!\nFile=%s, Function=%s, Line=%u, Expression=%s\n", FileName, FunctionName, LineNo, Expression);
		MessageBoxA(nullptr, "Assertion Faild!\n\nSee console for detail.", "Fatal Error", MB_OK | MB_ICONERROR);

		assert(false);
	}

	class component final : public component_interface
	{
	public:
		void post_start()
		{
			utils::nt::library avs2core{ "avs2-core.dll" };
			utils::hook::jump(avs2core.get_proc<void*>("XCgsqzn0000176"), avs2_log, true);
			utils::hook::iat(utils::nt::library{}, "user32.dll", "MessageBoxA", msgbox_hook);

			utils::hook::jump(0x14052838C, on_assert);
		}
	};
}

REGISTER_COMPONENT(logger::component)
