#include <std_include.hpp>
#include "launcher.hpp"

#include <utils/nt.hpp>
#include <utils/smbios.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>
#include <webview.h>
#include <json.hpp>
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

	webview::webview _main_window(true, nullptr);
	_main_window.set_title("Laochan-Eacnet Infinitas");
	_main_window.set_size(750, 480, WEBVIEW_HINT_NONE);

	//传过来的req是json
	_main_window.bind("exit", [&](const std::string& req) -> std::string {
		_main_window.terminate();
		return "";
		});


	_main_window.bind("setToken", [&](const std::string& req) -> std::string {
		try
		{
			json data = json::parse(req);
			if (!data.is_array() || !data.begin().value().is_string())
			{
				json result = { {"result","error"},{"info","first param must be string"} };
				return result.dump();
			}
			launcher::token = data.begin().value();
		}
		catch (const std::exception& e)
		{
			json result = { {"result","error"},{"info",e.what()}};
			return result.dump();
		}
		json result = { {"result","OK"} };
		return result.dump();
		});

	_main_window.bind("setServerAddress", [&](const std::string& req) -> std::string {
		try
		{
			json data = json::parse(req);
			if (!data.is_array() || !data.begin().value().is_string())
			{
				json result = { {"result","error"},{"info","first param must be string"} };
				return result.dump();
			}
			launcher::get_service_address = data.begin().value();
		}
		catch (const std::exception& e)
		{
			json result = { {"result","error"},{"info",e.what()} };
			return result.dump();
		}
		json result = { {"result","OK"} };
		return result.dump();
		});


	_main_window.bind("setAsioDeviceName", [&](const std::string& req) -> std::string {
		try
		{
			json data = json::parse(req);
			if (!data.is_array() || !data.begin().value().is_string())
			{
				json result = { {"result","error"},{"info","first param must be string"} };
				return result.dump();
			}
			launcher::asio_device_name = data.begin().value();
		}
		catch (const std::exception& e)
		{
			json result = { {"result","error"},{"info",e.what()} };
			return result.dump();
		}
		json result = { {"result","OK"} };
		return result.dump();
		});

	_main_window.bind("setDisplayMode", [&](const std::string& req) -> std::string {
		try
		{
			json data = json::parse(req);
			if (!data.is_array() || !data.begin().value().is_number_integer())
			{
				json result = { {"result","error"},{"info","first param must be integer"} };
				return result.dump();
			}
			launcher::disp_mode = static_cast<display_mode>(data.begin().value());
		}
		catch (const std::exception& e)
		{
			json result = { {"result","error"},{"info",e.what()} };
			return result.dump();
		}
		json result = { {"result","OK"} };
		return result.dump();
		});

	_main_window.bind("setSoundMode", [&](const std::string& req) -> std::string {
		try
		{
			json data = json::parse(req);
			if (!data.is_array() || !data.begin().value().is_number_integer())
			{
				json result = { {"result","error"},{"info","first param must be integer"} };
				return result.dump();
			}
			launcher::snd_mode = static_cast<sound_mode>(data.begin().value());
		}
		catch (const std::exception& e)
		{
			json result = { {"result","error"},{"info",e.what()} };
			return result.dump();
		}
		json result = { {"result","OK"} };
		return result.dump();
		});

	_main_window.bind("saveConfig", [&](const std::string& req) -> std::string {
		try
		{
			json data = json::parse(req);
			if (!data.is_array() || !data.begin().value().is_string())
			{
				json result = { {"result","error"},{"info","first param must be string"} };
				return result.dump();
			}
			utils::io::write_file("laochan-config.json", data.begin().value());
		}
		catch (const std::exception& e)
		{
			json result = { {"result","error"},{"info",e.what()} };
			return result.dump();
		}
		json result = { {"result","OK"} };
		return result.dump();
		});




	//this->main_window_.set_callback([](window* window, const UINT message, const WPARAM w_param, const LPARAM l_param) -> LRESULT
	//{
	//	if (message == WM_CLOSE)
	//	{
	//		window::close_all();
	//		ExitProcess(0);
	//	}

	//	return DefWindowProcA(*window, message, w_param, l_param);
	//});


	auto inject = "<script>window.uuid='" + utils::string::dump_hex(utils::smbios::get_uuid(), "") + "';</script>";

	if (utils::io::file_exists("laochan-config.json")) {
		inject += "<script>window.settings=" + utils::io::read_file("laochan-config.json") + ";</script>";
	}
	_main_window.set_html(inject + load_content(MENU_MAIN));
	_main_window.run();
}

void launcher::run() const
{
	//window::run();
}

std::string launcher::load_content(const int res)
{
	return utils::nt::load_resource(launcher::dll_module, res);
}
