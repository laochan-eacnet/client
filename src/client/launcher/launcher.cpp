#include <std_include.hpp>

#include "launcher.hpp"
#include "launcher_module.hpp"

#include <utils/nt.hpp>
#include <utils/smbios.hpp>
#include <utils/string.hpp>
#include <utils/hook.hpp>
#include <utils/io.hpp>

#include <steam/steam.hpp>
#include <game/game.hpp>

#include "component/steam_proxy.hpp"

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
	smartview_->set_dpi_aware_size(1280, 720);

	smartview_->expose("close", [this]()
		{
			smartview_->close();
		}
	);

	smartview_->expose("minimize", [this]()
		{
			smartview_->set_minimized(true);
		}
	);

	smartview_->expose("mounted", [this]()
		{
			smartview_->show();
		}
	);

	smartview_->expose("shellExecute", [](std::string command, std::string args)
		{
			ShellExecuteA(nullptr, "open", command.data(), args.data(), nullptr, 1);
		}, false
	);

	smartview_->expose("detectGameInstall", [](int game_index) -> std::vector<std::string>
		{
			::game::environment::set_game(static_cast<game>(game_index));

			try
			{
				auto install = ::game::environment::get_install_path().string();
				auto resource = ::game::environment::get_resource_path().string();

				return { install, resource };
			}
			catch (std::exception)
			{

			}

			return {};
		}
	);

	smartview_->expose("readFile", [](std::string path) -> std::string 
		{
			if (!std::filesystem::exists(path))
				return "<NOT EXIST>";

			FILE *file = nullptr;
			fopen_s(&file, path.data(), "r");
			fseek(file, 0, SEEK_END);
			auto size = ftell(file);
			fseek(file, 0, SEEK_SET);

			std::string buffer;
			buffer.resize(size + 1);
			fread(buffer.data(), 1, size, file);
			buffer.resize(size);

			fclose(file);

			return buffer;
		}, true
	);

	smartview_->expose("writeFile", [](std::string path, std::string content) -> void
		{
			if (std::filesystem::exists(path))
				std::filesystem::remove(path);

			FILE* file = nullptr;
			fopen_s(&file, path.data(), "w");
			fwrite(content.data(), 1, content.size(), file);
			fclose(file);
		}
	);

	smartview_->expose("uuid", []() -> std::string
		{
			const auto steamid = steam_proxy::get_steam_id();
			if (steamid != 0xFFFFFFFFDEADBEEFul)
				return std::to_string(steamid);

			const auto uuid = utils::string::dump_hex(utils::smbios::get_uuid(), "");
			return uuid;
		}
	);

#ifdef _DEBUG
	smartview_->set_url("http://localhost:5173/");
	smartview_->set_dev_tools(true);
#else
	// spa hack
	smartview_->embed(([]
		{
			auto resources = saucer::embedded::all();
			resources.emplace("", resources["index.html"]);

			return resources;
		}
	)());

	smartview_->serve("");
#endif
}

bool launcher::run() const
{
	smartview_->run();
	return true;
}

std::string launcher::load_content(const int res)
{
	return utils::nt::load_resource(GetModuleHandle(nullptr), res);
}
