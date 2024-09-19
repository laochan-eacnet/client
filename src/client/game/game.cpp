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

		//https://learn.microsoft.com/zh-cn/archive/msdn-magazine/2017/may/c-use-modern-c-to-access-the-windows-registry
		std::wstring RegGetString(HKEY hKey, const std::wstring& subKey,
			const std::wstring& value) {
			// Retrieve the size of the string value data
			DWORD dataSize{};
			auto m_errorCode = ::RegGetValueW(hKey, subKey.c_str(), value.c_str(),
				RRF_RT_REG_SZ, nullptr, nullptr, &dataSize);

			// Check if the value information retrieval was successful
			if (m_errorCode != ERROR_SUCCESS) {
				throw std::exception("Cannot read string from registry", m_errorCode);
			}

			// Use smart pointers for automatic memory management
			std::vector<wchar_t> data(dataSize / sizeof(wchar_t));

			// Retrieve the actual string value data
			m_errorCode = ::RegGetValueW(hKey, subKey.c_str(), value.c_str(),
				RRF_RT_REG_SZ, nullptr, data.data(), &dataSize);

			// Check if the value data retrieval was successful
			if (m_errorCode != ERROR_SUCCESS) {
				throw std::exception("Cannot read string from registry", m_errorCode);
			}

			// Resize the vector based on the actual size of the string
			data.resize(dataSize / sizeof(wchar_t) - 1);

			// Convert the wchar_t vector to a wstring and return
			return std::wstring(data.begin(), data.end());
		}

		static std::wstring get_reg_value(launcher::game game_type, std::wstring const& query)
		{
			auto path = L"SOFTWARE\\KONAMI\\" + get_stringw(game_type);
			auto value = RegGetString(HKEY_LOCAL_MACHINE, path, query);
			return value;
		}

		std::wstring get_reg_value(std::wstring const& query) {
			return get_reg_value(get_game(), query);
		}

		std::filesystem::path get_install_path()
		{
			return get_reg_value(L"InstallDir");
		}

		std::filesystem::path get_resource_path()
		{
			return get_reg_value(L"ResourceDir");
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
			{
				return;
			}
			this->game_type = gametype;
			try
			{
				install_path = game::environment::get_install_path(gametype);
				resource_path = game::environment::get_resource_path(gametype);
				switch (gametype)
				{
				case launcher::game::iidx:
				{
					game_module_path = install_path / L"game" / L"app" / L"bm2dx.exe";
					settings_module_path = install_path / L"launcher" / L"modules" / L"bm2dx_settings.exe";
					updater_module_path = install_path / L"launcher" / L"modules" / L"bm2dx_updater.exe";
					game_module_target_version = IIDX_TARGET_VERSIONW;
					break;
				}
				case launcher::game::sdvx:
				{
					game_module_path = install_path / L"game" / L"modules" / L"sv6c.exe";
					settings_module_path = install_path / L"launcher" / L"modules" / L"settings.exe";
					updater_module_path = install_path / L"launcher" / L"modules" / L"updater.exe";
					game_module_target_version = SDVX_TARGET_VERSIONW;
					break;
				}
				case launcher::game::gitadora:
				{
					game_module_path = install_path / L"game" / L"modules" / L"gitadora.exe";
					settings_module_path = install_path / L"launcher" / L"modules" / L"settings.exe";
					updater_module_path = install_path / L"launcher" / L"modules" / L"updater.exe";
					game_module_target_version = GITADORA_TARGET_VERSIONW;
					break;
				}
				default:
				{
					throw std::exception("unexcepted gametype");
					break;
				}
				}
				game_module_version = utils::nt::library::get_version(game_module_path.wstring());
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
		std::wstring gamemeta::get_install_path()
		{
			return install_path.wstring();
		}
		std::wstring gamemeta::get_resource_path()
		{
			return resource_path.wstring();
		}
		std::wstring gamemeta::get_game_module_path()
		{
			return game_module_path.wstring();
		}
		std::wstring gamemeta::get_settings_module_path()
		{
			return settings_module_path.wstring();
		}
		std::wstring gamemeta::get_updater_module_path()
		{
			return updater_module_path.wstring();
		}
		std::wstring gamemeta::get_game_module_version()
		{
			return game_module_version;
		}
		std::wstring gamemeta::get_game_module_target_version()
		{
			return game_module_target_version;
		}
}
}
