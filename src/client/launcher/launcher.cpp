#include <std_include.hpp>
#include "launcher.hpp"

#include <utils/nt.hpp>
#include <utils/smbios.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>

using json = nlohmann::json;

std::string launcher::token;
std::string launcher::get_service_address;
std::string launcher::asio_device_name;
launcher::display_mode launcher::disp_mode;
launcher::sound_mode launcher::snd_mode;
HMODULE launcher::dll_module;

launcher::launcher()
{
	this->create_main_menu();
}

void launcher::create_main_menu()
{
	_main_window = std::make_unique<webview::webview>(true, nullptr);

	_main_window->set_title("Laochan-Eacnet Infinitas");
	_main_window->set_size(750, 480, WEBVIEW_HINT_NONE);

	_main_window->bind("exit", [this](auto) -> std::string {
		_main_window->terminate();
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
}

void launcher::run() const
{
	_main_window->run();
}

std::string launcher::load_content(const int res)
{
	return utils::nt::load_resource(launcher::dll_module, res);
}
