#include <std_include.hpp>
#include "game.hpp"

namespace game
{
	namespace environment
	{
		launcher::game game = launcher::game::invalid;
		utils::nt::library game_module;
		bool inited = false;;

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
			switch (current_game)
			{
			case launcher::game::iidx:
				return "beatmania IIDX INFINITAS";

			case launcher::game::sdvx:
				return "SOUND VOLTEX EXCEED GEAR";

			case launcher::game::gitadora:
				return "GITADORA";

			default:
				return "Unknown (" + std::to_string(static_cast<int>(current_game)) + ")";
			}
		}

		std::string get_version()
		{
			if (!inited)
				throw std::runtime_error{"Game environment not ready"};

			static auto version = game_module.get_version();
			return version;
		}

		std::string get_reg_value(std::string const &query)
		{
			HKEY key;
			auto path = "SOFTWARE\\KONAMI\\" + get_string();

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

		std::filesystem::path get_install_path()
		{
			return get_reg_value("InstallDir");
		}

		std::filesystem::path get_resource_path()
		{
			return get_reg_value("ResourceDir");
		}
	}
}
