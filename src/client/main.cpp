#include <std_include.hpp>
#include "launcher/launcher.hpp"
#include "loader/loader.hpp"
#include "loader/component_loader.hpp"
#include "component/steam_proxy.hpp"

#include <utils/nt.hpp>
#include <utils/hook.hpp>
#include <utils/flags.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>

#include <game/game.hpp>


void enable_dpi_awareness()
{
	const utils::nt::library user32{ "user32.dll" };
	const auto set_dpi = user32
		? user32.get_proc<BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT)>("SetProcessDpiAwarenessContext")
		: nullptr;
	if (set_dpi)
	{
		set_dpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	}
}

DECLSPEC_NORETURN void WINAPI exit_hook(const int code)
{
	component_loader::pre_destroy();
	ExitProcess(code);
}

int dump_mount_point()
{
	component_loader::post_avs_init();
	return avs2::fs_dump_mountpoint();
}

HMODULE WINAPI get_module_handle_a(LPCSTR lpModuleName)
{
	if (!lpModuleName)
		return game::environment::get_module();

	return GetModuleHandleA(lpModuleName);
}

HMODULE WINAPI get_module_handle_w(LPCWSTR lpModuleName)
{
	if (!lpModuleName)
		return game::environment::get_module();

	return GetModuleHandleW(lpModuleName);
}

DWORD WINAPI get_module_file_name_a(HMODULE hModule, LPSTR lpFilename, DWORD nSize)
{
	if (!hModule)
		hModule = game::environment::get_module();

	return GetModuleFileNameA(hModule, lpFilename, nSize);
}

FARPROC load_binary(const launcher::game game)
{
	loader loader;
	utils::nt::library self;

	loader.set_import_resolver([self](const std::string& library, const std::string& function) -> void* {
		// dump mount point
		if (library == "avs2-core.dll" && function == "#105")
		{
			return dump_mount_point;
		}
		else if (function == "ExitProcess")
		{
			return exit_hook;
		}
		else if (function == "GetModuleHandleA")
		{
			return get_module_handle_a;
		}
		else if (function == "GetModuleHandleW")
		{
			return get_module_handle_w;
		}
		else if (function == "GetModuleFileNameA")
		{
			return get_module_file_name_a;
		}

		return component_loader::load_import(library, function);
	});

	std::string binary;
	switch (game)
	{
		case launcher::game::iidx:
			binary = "bm2dx.exe";
			break;
		case launcher::game::sdvx:
			binary = "sv6c.exe";
			break;
		case launcher::game::gitadora:
			binary = "gitadora.exe";
			break;
		default:
			throw std::runtime_error("Unsupported game!");
	}

	auto mod = loader.load_library(binary);
	game::environment::set_module(mod);

	printf("version %s\n", mod.get_version().data());

	return mod.get_entry_point();
}

bool try_set_game_environment(launcher::game game)
{
	game::environment::set_game(game);

	try
	{
		auto game_path = game::environment::get_install_path() / "game";
		auto modules_path = game_path / (game == launcher::game::iidx ? "app" : "modules");

		SetCurrentDirectoryW(game_path.wstring().data());
		AddDllDirectory(modules_path.wstring().data());

		return true;
	}
	catch (std::exception&)
	{

	}

	return false;
}

launcher::game detect_game_from_arguments()
{
	if (utils::flags::has_flag("iidx"))
	{
		return launcher::game::iidx;
	}

	if (utils::flags::has_flag("sdvx"))
	{
		return launcher::game::sdvx;
	}

	if (utils::flags::has_flag("gitadora"))
	{
		return launcher::game::gitadora;
	}

	return launcher::game::invalid;
}


static void set_qos_mode(bool high)
{
	static const bool can_set_qos_mode = [] {
		utils::nt::library ntdll{ "ntdll.dll" };

		OSVERSIONINFOEXW info;
		ntdll.invoke_pascal<NTSTATUS>("RtlGetVersion", &info);

		return info.dwMajorVersion >= 10 && info.dwBuildNumber >= 16299;
	}();

	if (!can_set_qos_mode)
		return;

	auto self = GetCurrentProcess();

	MEMORY_PRIORITY_INFORMATION priority_info{
		.MemoryPriority = high ? 5U : 3U,
	};

	PROCESS_POWER_THROTTLING_STATE power_state{
		.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION,
		.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED,
		.StateMask = high,
	};

	SetProcessInformation(self, ProcessPowerThrottling, &priority_info, sizeof(PROCESS_POWER_THROTTLING_STATE));
	SetProcessInformation(self, ProcessMemoryPriority, &power_state, sizeof(MEMORY_PRIORITY_INFORMATION));
	SetPriorityClass(self, IDLE_PRIORITY_CLASS);
}

int main()
{
	FARPROC entry_point;
	enable_dpi_awareness();
	set_qos_mode(false);

	// pin system dinput8 here to prevent old client from loading
	LoadLibraryA("dinput8.dll");

	std::srand(uint32_t(time(nullptr)));

	{
		auto premature_shutdown = true;
		const auto _ = gsl::finally([&premature_shutdown] {
			component_loader::pre_destroy();
		});

		try
		{
			auto game = detect_game_from_arguments();

			if (game == launcher::game::invalid)
			{
				const launcher launcher;
				game = launcher.run();
			}

			if (game == launcher::game::invalid)
			{
				return 0;
			}

			set_qos_mode(true);
			try_set_game_environment(game);

			component_loader::create_components(game::environment::get_game());

			if (!component_loader::post_start())
				return 0;

			entry_point = load_binary(game::environment::get_game());

			if (!entry_point)
				throw std::runtime_error{ "Unable to load binary into memory" };

			if (!component_loader::post_load())
				return 0;

			steam_proxy::set_status(game::environment::get_string());
		}
		catch (std::exception& e)
		{
			MessageBoxA(nullptr, e.what(), "Laochan Bootstrap ERROR", MB_ICONERROR);
			return 1;
		}
	}

	return static_cast<int>(entry_point());
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	return main();
}
