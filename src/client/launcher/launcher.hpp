#pragma once

class launcher final
{
public:
	enum class display_mode: uint32_t
	{
		fullscreen,
		windowed_720p,
		windowed_900p,
		windowed_1080p,
	};

	enum class sound_mode : uint32_t
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
	std::unique_ptr<webview::webview> _main_window;
	void create_main_menu();
	static std::string load_content(int res);
};
