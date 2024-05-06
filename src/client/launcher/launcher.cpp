#include <std_include.hpp>
#include "launcher.hpp"

#include <utils/nt.hpp>
#include <utils/smbios.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>
#include <Windows.h>

#include <steam/steam.hpp>

#include "component/steam_proxy.hpp"

using json = nlohmann::json;

std::string launcher::token;
std::string launcher::get_service_address;
std::string launcher::asio_device_name;
launcher::display_mode launcher::disp_mode;
launcher::sound_mode launcher::snd_mode;
HMODULE launcher::dll_module;

launcher::launcher()
{
	this->run_game = false;
	this->create_main_menu();
}

void launcher::create_main_menu()
{
	_main_window = std::make_unique<webview::webview>(true, launcher::dll_module);
	_main_window->set_title("Laochan-Eacnet Infinitas Launcher");
	_main_window->set_size(900, 480, WEBVIEW_HINT_NONE);

	_main_window->bind("getAsioDeviceList", [this](auto)->std::string
		{
			HKEY key;
			if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\ASIO", 0, KEY_READ, &key))
				return "[\"ASIO DEVICE NOT FOUND\"]";

			DWORD subkey_counts, subkey_name_maxlen;
			RegQueryInfoKeyA(key, nullptr, nullptr, nullptr, &subkey_counts, &subkey_name_maxlen, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

			auto result = json::array();

			char* buffer = utils::memory::allocate<char>(subkey_name_maxlen * 2);

			for (DWORD i = 0; i < subkey_counts; i++)
			{
				DWORD size = subkey_name_maxlen * 2;
				RegEnumKeyExA(key, i, buffer, &size, nullptr, nullptr, nullptr, nullptr);

				result.push_back(std::string{ buffer });
			}

			utils::memory::free(buffer);
			return result.dump();
		});

	_main_window->bind("getVersion", [this](auto)->std::string
		{
			return json{ VERSION }.dump();
		});

	_main_window->bind("startGame", [this](auto) -> std::string
		{
			run_game = true;

			_main_window->terminate();
			return {};
		});

	_main_window->bind("showWindow", [this](auto) -> std::string
		{
			std::thread{
				&launcher::wait_and_show_window,
				HWND(_main_window->window().value()), 16ms
			}.detach();

			return {};
		});

	_main_window->bind("hideWindow", [this](auto) -> std::string
		{
			ShowWindow(reinterpret_cast<HWND>(_main_window->window().value()), SW_HIDE);
			return {};
		});

	_main_window->bind("createSettingsProcess", [this](auto) -> std::string
		{
			STARTUPINFO si = { 0 };
			PROCESS_INFORMATION pi = { 0 };
			si.cb = sizeof(si);
			si.wShowWindow = SW_SHOW;

			utils::nt::library bm2dx{};
			auto settings_path = std::filesystem::path{
				bm2dx.get_folder() + "/../../launcher/modules/bm2dx_settings.exe"
			}.make_preferred().string();

			CreateProcessA(
				settings_path.data(), nullptr, 0, 0, false, 0,
				nullptr, nullptr, &si, &pi
			);

			return {};
		});

	_main_window->bind("uuid", [this](auto) -> std::string
		{
			const auto steamid = steam_proxy::get_steam_id();
			if (steamid != 0xFFFFFFFFDEADBEEFul)
				return json{ std::to_string(steamid) }.dump();

			const auto uuid = utils::string::dump_hex(utils::smbios::get_uuid(), "");
			return json{ uuid }.dump();
		});

	_main_window->bind("getOptions", [this](auto) -> std::string
		{
			if (!utils::io::file_exists("laochan-config.json"))
				return "null";

			return utils::io::read_file("laochan-config.json");
		});

	_main_window->bind("setOptions", [this](auto param_json) -> std::string
		{
			const json params = json::parse(param_json);

			if (!params.is_array() || !params.size())
				return {};

			const auto options = params[0];

			if (!options.is_object())
				return {};

			if (options["token"].is_string())
				launcher::token = options.value("token", "");

			if (options["serverUrl"].is_string())
				launcher::get_service_address = options.value("serverUrl", "");

			if (options["asioDevice"].is_string())
				launcher::asio_device_name = options.value("asioDevice", "");

			if (options["displayMode"].is_number_integer())
				launcher::disp_mode = options.value<launcher::display_mode>("displayMode", launcher::display_mode::windowed_720p);

			if (options["soundMode"].is_number_integer())
				launcher::snd_mode = options.value<launcher::sound_mode>("soundMode", launcher::sound_mode::wasapi);

			utils::io::write_file("laochan-config.json", options.dump());
			return {};
		});

	_main_window->set_html(load_content(MENU_MAIN));
}

void launcher::wait_and_show_window(HWND hwnd, std::chrono::milliseconds msec)
{
	std::this_thread::sleep_for(msec);

	ShowWindow(hwnd, SW_SHOW);
	steam_proxy::set_status("\xF0\x9F\x97\xBF Laochan Launcher");
}

bool launcher::run() const
{
	_main_window->run();

	if (run_game)
	{
		steam_proxy::set_status("\xF0\x9F\x97\xBF beatmania IIDX INFINITAS");
	}

	return run_game;
}

std::string launcher::load_content(const int res)
{
	return utils::nt::load_resource(launcher::dll_module, res);
}
