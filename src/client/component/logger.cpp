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

    static BOOL output_callback(char* buf, DWORD size, HANDLE file) 
    {
        // check size
        if (size == 0) {
            return TRUE;
        }

        // state machine for parsing style and converting to CRLF
        // this is needed because newer AVS buffers multiple lines unlike the old callback
        static auto handle = GetStdHandle(STD_OUTPUT_HANDLE);
        static WORD last_style = log_level_color(3);
        static WORD new_style = last_style;
        static size_t position = 0;
        for (size_t i = 0; i < size; i++) {
            switch (position) {
            case 0: {
                if (buf[i] == ']')
                    position++;
                else
                    position = 0;
                break;
            }
            case 1: {
                if (buf[i] == ' ')
                    position++;
                else
                    position = 0;
                break;
            }
            case 2: {
                switch (buf[i]) {
                case 'M':
                    new_style = log_level_color(4);
                    break;
                case 'I':
                    new_style = log_level_color(3);
                    break;
                case 'W':
                    new_style = log_level_color(2);
                    break;
                case 'F':
                    new_style = log_level_color(1);
                    break;
                default:
                    position = 0;
                    break;
                }
                if (position > 0)
                    position++;
                break;
            }
            case 3: {
                position = 0;
                if (buf[i] == ':') {
                    last_style = new_style;

                    // flush line
                    for (size_t j = i + 1; j < size; j++) {
                        if (buf[j] == '\n') {
                            buf[j] = 0;
                            SetConsoleTextAttribute(handle, last_style);
                            printf("%s\n", buf);
                            buf = &buf[j + 1];
                            size -= j + 1;
                            i = 0;
                            break;
                        }
                    }
                }
                break;
            }
            default:
                position = 0;
                break;
            }
        }

        // push rest to logger
        if (size > 1) {
            SetConsoleTextAttribute(handle, last_style);
            printf("%s", buf);
        }

        // success
        return TRUE;
    }

	class component final : public component_interface
	{
	public:
		void post_start()
		{
			// set log level
			utils::hook::inject(0x140002742 + 3, "misc");
            utils::hook::inject(0x14000289F + 3, output_callback);
		}
	};
}

REGISTER_COMPONENT(logger::component)
