#include <std_include.hpp>
#include "steam.hpp"

namespace steam
{
	const char* get_steam_install_path()
	{
		static std::string install_path{};
		if (!install_path.empty())
		{
			return install_path.data();
		}

		HKEY reg_key;
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\WOW6432Node\\Valve\\Steam", 0, KEY_QUERY_VALUE,
		                  &reg_key) ==
			ERROR_SUCCESS)
		{
			char path[MAX_PATH] = {0};
			DWORD length = sizeof(path);
			RegQueryValueExA(reg_key, "InstallPath", nullptr, nullptr, reinterpret_cast<BYTE*>(path),
			                 &length);
			RegCloseKey(reg_key);

			install_path = path;
		}

		return install_path.data();
	}
}
