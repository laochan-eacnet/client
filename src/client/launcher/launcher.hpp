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

	enum class game : size_t
	{
		invalid = -1,

		iidx,
		sdvx,
		gitadora,
		
		count,
		all = count,
	};

	launcher();

	bool run() const;
private:
	std::unique_ptr<webview::webview> _main_window;
	bool run_game;

	void create_main_menu();

	static void wait_and_show_window(HWND hwnd, std::chrono::milliseconds msec);
	static std::string load_content(int res);
};
