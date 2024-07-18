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

launcher::launcher()
{
	smartview_ = borderless_smartview_ptr(
		new borderless_smartview(
			saucer::options
			{
				.chrome_flags =
				{
					"--disable-gpu-vsync",
					"--disable-frame-rate-limit",
				}
			}
		), [](borderless_smartview* smartview) 
		{
			delete smartview;
		}
	);

	this->create_main_menu();
}

void launcher::create_main_menu()
{
	smartview_->set_title("Laochan-Eacnet Launcher");

	smartview_->expose("close", [this]()
		{
			smartview_->close();
		});

#ifdef _DEBUG
	smartview_->set_url("http://localhost:5173/");
#else
	// spa hack
	smartview_->embed(([]
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
	smartview_->show();
	smartview_->set_dev_tools(true);
	smartview_->run();
	return true;
}

std::string launcher::load_content(const int res)
{
	return utils::nt::load_resource(GetModuleHandle(nullptr), res);
}
