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

		bool is_ddr()
		{
			return get_game() == launcher::game::ddr;
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

		std::wstring get_stringw()
		{
			const auto current_game = get_game();
			return get_stringw(current_game);
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

				case launcher::game::ddr:
					return "DanceDanceRevolution";

				default:
					return "Unknown (" + std::to_string(static_cast<int>(game_type)) + ")";
			}
		}

		std::wstring get_stringw(launcher::game game_type)
		{
			switch (game_type)
			{
				case launcher::game::iidx:
					return L"beatmania IIDX INFINITAS";

				case launcher::game::sdvx:
					return L"SOUND VOLTEX EXCEED GEAR";

				case launcher::game::gitadora:
					return L"GITADORA";

				case launcher::game::ddr:
					return L"DanceDanceRevolution";

				default:
					return L"Unknown (" + std::to_wstring(static_cast<int>(game_type)) + L")";
			}
		}

		std::string get_version()
		{
			if (!inited)
				throw std::runtime_error{ "Game environment not ready" };
			static auto version = game_module.get_version();
			return version;
		}

		static std::wstring get_reg_value(launcher::game game_type, std::wstring const& query)
		{
			HKEY key;
			auto path = L"SOFTWARE\\KONAMI\\" + get_stringw(game_type);

			if (RegOpenKeyExW(HKEY_CURRENT_USER, path.data(), 0, KEY_READ | KEY_WOW64_64KEY, &key)
				&& RegOpenKeyExW(HKEY_LOCAL_MACHINE, path.data(), 0, KEY_READ | KEY_WOW64_64KEY, &key))
			{
				throw std::exception("Game is not installed properly (key is not found)");
			}

			DWORD type, size;

			if (RegQueryValueExW(key, query.data(), 0, &type, 0, &size))
			{
				throw std::exception("Game is not installed properly (failed to get key value size)");
			}

			if (type != 1)
			{
				throw std::exception("Game is not installed properly (illegal key value type)");
			}

			std::wstring value;
			value.resize(size / sizeof(wchar_t) - 1);

			if (RegQueryValueExW(key, query.data(), 0, &type, reinterpret_cast<LPBYTE>(value.data()), &size))
			{
				RegCloseKey(key);
				throw std::exception("failed to read key value");
			}

			RegCloseKey(key);

			return value;
		}

		std::filesystem::path get_install_path()
		{
			return get_install_path(get_game());
		}

		std::filesystem::path get_resource_path()
		{
			return get_resource_path(get_game());
		}

		std::filesystem::path get_install_path(launcher::game game_type)
		{
			return get_reg_value(game_type, L"InstallDir");
		}

		std::filesystem::path get_resource_path(launcher::game game_type)
		{
			return get_reg_value(game_type, L"ResourceDir");
		}

		void gamemeta::ensure_created(launcher::game gametype)
		{
			if (inited)
				return;

			this->game_type = gametype;

			try
			{
				install_path = game::environment::get_install_path(gametype);
				resource_path = game::environment::get_resource_path(gametype);
				switch (gametype)
				{
					case launcher::game::iidx:
					{
                        game_module_name = "bm2dx.exe";

						if (std::filesystem::exists("bm2dx.exe"))
							game_module_path = std::filesystem::absolute("bm2dx.exe");
						else
							game_module_path = install_path / L"game" / L"app" / L"bm2dx.exe";

						settings_module_path = install_path / L"launcher" / L"modules" / L"bm2dx_settings.exe";
						updater_module_path = install_path / L"launcher" / L"modules" / L"bm2dx_updater.exe";
						game_module_target_version = IIDX_TARGET_VERSION;
						break;
					}
					case launcher::game::sdvx:
					{
                        game_module_name = "sv6c.exe";

						if (std::filesystem::exists("sv6c.exe"))
							game_module_path = std::filesystem::absolute("sv6c.exe");
						else
							game_module_path = install_path / L"game" / L"modules" / L"sv6c.exe";

						settings_module_path = install_path / L"launcher" / L"modules" / L"settings.exe";
						updater_module_path = install_path / L"launcher" / L"modules" / L"updater.exe";
						game_module_target_version = SDVX_TARGET_VERSION;
						break;
					}
					case launcher::game::gitadora:
					{
                        game_module_name = "gitadora.exe";

						if (std::filesystem::exists("gitadora.exe"))
							game_module_path = std::filesystem::absolute("gitadora.exe");
						else
							game_module_path = install_path / L"game" / L"modules" / L"gitadora.exe";

						settings_module_path = install_path / L"launcher" / L"modules" / L"settings.exe";
						updater_module_path = install_path / L"launcher" / L"modules" / L"updater.exe";
						game_module_target_version = GITADORA_TARGET_VERSION;
						break;
					}
					case launcher::game::ddr:
					{
						game_module_name = "ddr-konaste.exe";

						if (std::filesystem::exists("ddr-konaste.exe"))
							game_module_path = std::filesystem::absolute("ddr-konaste.exe");
						else
							game_module_path = install_path / L"game" / L"modules" / L"ddr-konaste.exe";

						settings_module_path = install_path / L"launcher" / L"modules" / L"settings.exe";
						updater_module_path = install_path / L"launcher" / L"modules" / L"updater.exe";
						game_module_target_version = DDR_TARGET_VERSION;
						break;
					}
					default:
					{
						throw std::exception("unexcepted gametype");
						break;
					}
				}

				game_module_version = utils::nt::library::get_version(game_module_path);
				installed = true;
			}
			catch (const std::exception)
			{
				installed = false;
			}
			inited = true;
		}

		static gamemeta gamemetas[static_cast<size_t>(launcher::game::count)];

		gamemeta gamemeta::get_gamemeta(launcher::game game_type)
		{
			gamemeta& gamemeta = gamemetas[static_cast<size_t>(game_type)];
			gamemeta.ensure_created(game_type);
			return gamemeta;
		}

		bool gamemeta::get_install_state()
		{
			return installed;
		}

		std::wstring gamemeta::get_install_path()
		{
			return install_path.generic_wstring();
		}

		std::wstring gamemeta::get_resource_path()
		{
			return resource_path.generic_wstring();
		}

		std::wstring gamemeta::get_game_module_path()
		{
			return game_module_path.generic_wstring();
		}

		std::wstring gamemeta::get_settings_module_path()
		{
			return settings_module_path.generic_wstring();
		}

		std::wstring gamemeta::get_updater_module_path()
		{
			return updater_module_path.generic_wstring();
		}

		std::string gamemeta::get_game_module_name()
		{
			return game_module_name;
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
