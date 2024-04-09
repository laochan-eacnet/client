#include <std_include.hpp>
#include "launcher.hpp"

#include <utils/nt.hpp>
#include <utils/smbios.hpp>
#include <utils/string.hpp>

std::string launcher::token;
std::string launcher::get_service_address;
launcher::display_mode launcher::disp_mode;
launcher::sound_mode launcher::snd_mode;
HMODULE launcher::dll_module;

launcher::launcher()
{
	this->create_main_menu();
}

void launcher::create_main_menu()
{
	this->main_window_.register_callback("run", [this](html_frame::callback_params* params)
	{
		this->main_window_.close();
	});

	this->main_window_.register_callback("setToken", [this](html_frame::callback_params* params)
	{
		if (params->arguments.empty()) return;

		const auto param = params->arguments[0];
		if (!param.is_string()) return;

		launcher::token = param.get_string();
	});

	this->main_window_.register_callback("setServerAddress", [this](html_frame::callback_params* params)
	{
		if (params->arguments.empty()) return;

		const auto param = params->arguments[0];
		if (!param.is_string()) return;

		launcher::get_service_address = param.get_string();
	});

	this->main_window_.register_callback("setDisplayMode", [this](html_frame::callback_params* params)
	{
		if (params->arguments.empty()) return;

		const auto param = params->arguments[0];
		if (!param.is_number()) return;

		launcher::disp_mode = static_cast<display_mode>(param.get_number());
	});

	this->main_window_.register_callback("setSoundMode", [this](html_frame::callback_params* params)
	{
		if (params->arguments.empty()) return;

		const auto param = params->arguments[0];
		if (!param.is_number()) return;

		launcher::snd_mode = static_cast<sound_mode>(param.get_number());
	});

	this->main_window_.set_callback([](window* window, const UINT message, const WPARAM w_param, const LPARAM l_param) -> LRESULT
	{
		if (message == WM_CLOSE)
		{
			window::close_all();
		}

		return DefWindowProcA(*window, message, w_param, l_param);
	});

	this->main_window_.create("Laochan-Eacnet Infinitas", 750, 480);

	auto uuidInject = "<script>window.uuid='" + utils::string::dump_hex(utils::smbios::get_uuid(), "") + "';</script>";
	this->main_window_.load_html(uuidInject + load_content(MENU_MAIN));
	this->main_window_.show();
}

void launcher::run() const
{
	window::run();
}

std::string launcher::load_content(const int res)
{
	return utils::nt::load_resource(launcher::dll_module, res);
}
