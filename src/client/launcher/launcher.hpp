#pragma once

class launcher final
{
public:
	launcher();

	bool run() const;

	static std::string token;
	static std::string get_service_address;
	static HMODULE dll_module;

	static const char* get_args();
private:
	std::unique_ptr<webview::webview> _main_window;
	bool run_game;

	void create_main_menu();

	static void wait_and_show_window(HWND hwnd, std::chrono::milliseconds msec);
	static std::string load_content(int res);
};
