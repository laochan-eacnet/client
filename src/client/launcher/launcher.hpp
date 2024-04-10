#pragma once
#include "html/html_window.hpp"

class launcher final
{
public:
	enum class display_mode
	{
		fullscreen,
		windowed_720p,
		windowed_900p,
		windowed_1080p,
	};

	enum class sound_mode
	{
		wasapi,
		asio,
	};

	launcher();

	void run() const;

	static std::string token;
	static std::string get_service_address;
	static display_mode disp_mode;
	static sound_mode snd_mode;
	static std::string asio_device_name;
	static HMODULE dll_module;
private:
	html_window main_window_;

	void create_main_menu();

	static std::string load_content(int res);
};
