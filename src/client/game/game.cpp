#include <std_include.hpp>
#include "game.hpp"

namespace game
{
	namespace environment
	{
		launcher::game game = launcher::game::invalid;
		utils::nt::library game_module;
		bool inited = false;

		launcher::game get_game()
		{
			return game;
		}

		bool is_iidx()
		{
			return get_game() == launcher::game::iidx;
		}

		bool is_sdvx()
		{
			return get_game() == launcher::game::sdvx;
		}

		bool is_gitadora()
		{
			return get_game() == launcher::game::gitadora;
		}

		void set_game(const launcher::game _game)
		{
			game = _game;
		}

		utils::nt::library& get_module()
		{
			return game_module;
		}

		void set_module(utils::nt::library& library)
		{
			inited = true;
			game_module = library;
		}

		std::string get_param(std::string const& key)
		{
			std::string result;
			auto size = GetEnvironmentVariableA(key.data(), nullptr, 0);

			if (!size)
				return result;

			result.resize(size);
			GetEnvironmentVariableA(key.data(), result.data(), size);
			result.resize(size - 1);

			return result;
		}

		void set_param(std::string const& key, std::string const& value)
		{
			SetEnvironmentVariableA(key.data(), value.data());
		}

		std::string get_string()
		{
			const auto current_game = get_game();
			return get_string(current_game);
		}

		std::string get_string(launcher::game game_type)
		{
			switch (game_type)
			{
			case launcher::game::iidx:
				return "beatmania IIDX INFINITAS";

			case launcher::game::sdvx:
				return "SOUND VOLTEX EXCEED GEAR";

			case launcher::game::gitadora:
				return "GITADORA";

			default:
				return "Unknown (" + std::to_string(static_cast<int>(game_type)) + ")";
			}
		}

		std::string get_version()
		{
			if (!inited)
				throw std::runtime_error{ "Game environment not ready" };
			static auto version = game_module.get_version();
			return version;
		}

		static std::string get_reg_value(launcher::game game_type, std::string const& query)
		{
			HKEY key;
			auto path = "SOFTWARE\\KONAMI\\" + get_string(game_type);

			if (RegOpenKeyExA(HKEY_CURRENT_USER, path.data(), 0, KEY_READ | KEY_WOW64_64KEY, &key)
				&& RegOpenKeyExA(HKEY_LOCAL_MACHINE, path.data(), 0, KEY_READ | KEY_WOW64_64KEY, &key))
			{
				throw std::exception("Game is not installed properly (key is not found)");
			}

			DWORD type, size;

			if (RegQueryValueExA(key, query.data(), 0, &type, 0, &size))
			{
				throw std::exception("Game is not installed properly (failed to get key value size)");
			}

			if (type != 1)
			{
				throw std::exception("Game is not installed properly (illegal key value type)");
			}

			std::string value;
			value.resize(size - 1);

			if (RegQueryValueExA(key, query.data(), 0, &type, reinterpret_cast<LPBYTE>(value.data()), &size))
			{
				RegCloseKey(key);
				throw std::exception("failed to read key value");
			}

			RegCloseKey(key);

			return value;
		}

		std::string get_reg_value(std::string const& query) {
			return get_reg_value(get_game(), query);
		}

		std::filesystem::path get_install_path()
		{
			return get_reg_value("InstallDir");
		}

		std::filesystem::path get_resource_path()
		{
			return get_reg_value("ResourceDir");
		}

		static std::filesystem::path get_install_path(launcher::game game_type)
		{
			return get_reg_value(game_type, "InstallDir");
		}

		static std::filesystem::path get_resource_path(launcher::game game_type)
		{
			return get_reg_value(game_type, "ResourceDir");
		}


		void gamemeta::ensure_created(launcher::game game_type)
		{
			if (inited)
			{
				return;
			}
			this->game_type = game_type;
			try
			{
				install_path = game::environment::get_install_path(game_type);
				resource_path = game::environment::get_resource_path(game_type);
				switch (game_type)
				{
				case launcher::game::iidx:
				{
					game_module_path = install_path / "game" / "app" / "bm2dx.exe";
					settings_module_path = install_path / "launcher" / "modules" / "bm2dx_settings.exe";
					updater_module_path = install_path / "launcher" / "modules" / "bm2dx_updater.exe";
					game_module_target_version = IIDX_TARGET_VERSION;
					break;
				}
				case launcher::game::sdvx:
				{
					game_module_path = install_path / "game" / "modules" / "sv6c.exe";
					settings_module_path = install_path / "launcher" / "modules" / "settings.exe";
					updater_module_path = install_path / "launcher" / "modules" / "updater.exe";
					game_module_target_version = SDVX_TARGET_VERSION;
					break;
				}
				case launcher::game::gitadora:
				{
					game_module_path = install_path / "game" / "modules" / "gitadora.exe";
					settings_module_path = install_path / "launcher" / "modules" / "settings.exe";
					updater_module_path = install_path / "launcher" / "modules" / "updater.exe";
					game_module_target_version = GITADORA_TARGET_VERSION;
					break;
				}
				default:
				{
					throw std::exception("unexcepted gametype");
					break;
				}
				}
				game_module_version = utils::nt::library::get_version(game_module_path.string());
				installed = true;
			}
			catch (const std::exception)
			{
				installed = false;
			}
			inited = true;
		}

		static gamemeta gamemetas[(unsigned long long)launcher::game::count];

		gamemeta gamemeta::get_gamemeta(launcher::game game_type)
		{
			gamemeta& gamemeta = gamemetas[(unsigned long long)game_type];
			gamemeta.ensure_created(game_type);
			return gamemeta;
		}
		bool gamemeta::get_install_state()
		{
			return installed;
		}
		std::string gamemeta::get_install_path()
		{
			return install_path.string();
		}
		std::string gamemeta::get_resource_path()
		{
			return resource_path.string();
		}
		std::string gamemeta::get_game_module_path()
		{
			return game_module_path.string();
		}
		std::string gamemeta::get_settings_module_path()
		{
			return settings_module_path.string();
		}
		std::string gamemeta::get_updater_module_path()
		{
			return updater_module_path.string();
		}
		std::string gamemeta::get_game_module_version()
		{
			return game_module_version;
		}
		std::string gamemeta::get_game_module_target_version()
		{
			return game_module_target_version;
		}
}
}
