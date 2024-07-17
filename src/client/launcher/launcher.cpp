#include <std_include.hpp>
#include "launcher.hpp"

#include <utils/nt.hpp>
#include <utils/smbios.hpp>
#include <utils/string.hpp>
#include <utils/io.hpp>
#include <Windows.h>

#include <steam/steam.hpp>

#pragma warning(push)
#pragma warning(disable: 5030)
#pragma warning(disable: 4189)
#include <saucer/smartview.hpp>
#pragma warning(pop)

#include "component/steam_proxy.hpp"

using json = nlohmann::json;

namespace
{
	saucer::smartview smartview;
}

std::string token;
std::string get_service_address;
std::string asio_device_name;

launcher::display_mode disp_mode;
launcher::sound_mode snd_mode;

launcher::game target_game;
HMODULE game_module;

launcher::launcher()
{
	this->create_main_menu();
}

void launcher::create_main_menu()
{
	smartview.set_title("Laochan-Eacnet Launcher");
	smartview.set_url("https://www.baidu.com");
}

bool launcher::run() const
{
	smartview.show();
	smartview.run();
	return true;
}

std::string launcher::load_content(const int res)
{
	return utils::nt::load_resource(GetModuleHandle(nullptr), res);
}
