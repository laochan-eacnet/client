#include <std_include.hpp>

#include "launcher.hpp"
#include "launcher_module.hpp"

#include <utils/nt.hpp>
#include <utils/smbios.hpp>
#include <utils/string.hpp>
#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <steam/steam.hpp>

#include "component/steam_proxy.hpp"

using json = nlohmann::json;

namespace
{
	saucer::smartview<saucer::default_serializer, launcher_module> smartview
	{
		{
			.chrome_flags =
			{
				"--disable-gpu-vsync",
				"--disable-frame-rate-limit",
			}
		}
	};
}

launcher::launcher()
{
	this->create_main_menu();
}

void launcher::create_main_menu()
{
	smartview.set_title("Laochan-Eacnet Launcher");

	smartview.expose("close", []()
		{
			smartview.close();
		});

#ifdef _DEBUG
	smartview.set_url("http://localhost:5173/");
#else
	// spa hack
	smartview.embed(([]
		{
			auto resources = saucer::embedded::all();
			resources.emplace("", resources["index.html"]);

			return resources;
		}
	)());

	smartview.serve("");
#endif
}

bool launcher::run() const
{
	smartview.show();
	smartview.set_dev_tools(true);
	smartview.run();
	return true;
}

std::string launcher::load_content(const int res)
{
	return utils::nt::load_resource(GetModuleHandle(nullptr), res);
}
