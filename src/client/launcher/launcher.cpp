#include <std_include.hpp>
#include "launcher.hpp"

#include <utils/nt.hpp>
#include <utils/smbios.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>
#include <Windows.h>

using json = nlohmann::json;

std::string launcher::token;
std::string launcher::get_service_address;
std::string launcher::asio_device_name;
launcher::display_mode launcher::disp_mode;
launcher::sound_mode launcher::snd_mode;
HMODULE launcher::dll_module;
bool launcher::resume;

launcher::launcher()
{
	this->create_main_menu();
}

void launcher::create_main_menu()
{
	_main_window = std::make_unique<webview::webview>(true, nullptr);

	ICoreWebView2* core;
	HWND window;

	reinterpret_cast<ICoreWebView2Controller*>(_main_window->browser_controller().value())->get_CoreWebView2(&core);
	window = reinterpret_cast<HWND>(_main_window->window().value());

	_main_window->set_title("Laochan-Eacnet Infinitas");
	_main_window->set_size(750, 480, WEBVIEW_HINT_NONE);

	_main_window->bind("startGame", [this](auto) -> std::string {
		launcher::resume = true;
		_main_window->terminate();
		return {};
		});

	_main_window->bind("showWindow", [this](auto) -> std::string {
		std::thread thr(&launcher::waitandshow, reinterpret_cast<HWND>(_main_window->window().value()), 16);
		thr.detach();
		return {};
		});

	_main_window->bind("hideWindow", [this](auto) -> std::string {
		ShowWindow(reinterpret_cast<HWND>(_main_window->window().value()), SW_HIDE);
		return {};
		});

	_main_window->bind("createSettingsProcess", [this](auto) -> std::string {
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		si.cb = sizeof(si);
		si.wShowWindow = SW_SHOW;
		char arg[] = "../../launcher/modules/bm2dx_settings.exe";
		CreateProcessA(NULL, arg, NULL, NULL, false, 0, NULL, NULL, &si, &pi);
		return {};
		});

	_main_window->bind("uuid", [this](auto) -> std::string {
		const auto uuid = utils::string::dump_hex(utils::smbios::get_uuid(), "");
		return json{ uuid }.dump();
		});

	_main_window->bind("getOptions", [this](auto) -> std::string {
		if (!utils::io::file_exists("laochan-config.json")) {
			return "null";
		}

		return utils::io::read_file("laochan-config.json");
		});

	_main_window->bind("setOptions", [this](auto param_json) -> std::string {
		const json params = json::parse(param_json);

		if (!params.is_array() || !params.size()) {
			return {};
		}

		const auto options = params[0];

		if (!options.is_object()) {
			return {};
		}

		if (options["token"].is_string()) {
			launcher::token = options.value("token", "");
		}

		if (options["serverUrl"].is_string()) {
			launcher::get_service_address = options.value("serverUrl", "");
		}

		if (options["asioDevice"].is_string()) {
			launcher::asio_device_name = options.value("asioDevice", "");
		}

		if (options["displayMode"].is_number_integer()) {
			launcher::disp_mode = options.value<launcher::display_mode>("displayMode", launcher::display_mode::windowed_720p);
		}

		if (options["soundMode"].is_number_integer()) {
			launcher::snd_mode = options.value<launcher::sound_mode>("soundMode", launcher::sound_mode::wasapi);
		}

		utils::io::write_file("laochan-config.json", options.dump());
		return {};
		});

	_main_window->set_html(load_content(MENU_MAIN));
	std::thread thr(&launcher::waitandshow, window, 5000);
	thr.detach();


}

void launcher::waitandshow(HWND hwnd, int milli) {
	std::chrono::duration<int, std::milli> duration1(milli);
	std::this_thread::sleep_for(duration1);
	ShowWindow(hwnd, SW_SHOW);
}

void launcher::run() const
{
	_main_window->run();

}

std::string launcher::load_content(const int res)
{
	return utils::nt::load_resource(launcher::dll_module, res);
}
