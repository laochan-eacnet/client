#include <std_include.hpp>
#include "launcher/launcher.hpp"
#include "loader/loader.hpp"
#include "loader/component_loader.hpp"

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
	exit(code);
}

void create_console()
{
	AllocConsole();

	FILE* f;
	auto _ = freopen_s(&f, "CONOUT$", "w+t", stdout);
	_ = freopen_s(&f, "CONOUT$", "w", stderr);
	_ = freopen_s(&f, "CONIN$", "r", stdin);

	SetConsoleCtrlHandler([](DWORD control_type) -> BOOL
		{
			if (control_type == CTRL_CLOSE_EVENT)
			{
				exit_hook(0);
			}

			return true;
		}, true);
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

DWORD
WINAPI
get_module_file_name_a(
	_In_opt_ HMODULE hModule,
	_Out_writes_to_(nSize, ((return < nSize) ? (return +1) : nSize)) LPSTR lpFilename,
	_In_ DWORD nSize
)
{
	if (!hModule)
		game::environment::get_module().get_path();

	return GetModuleFileNameA(hModule, lpFilename, nSize);
}

FARPROC load_binary(const launcher::game game)
{
	loader loader;
	utils::nt::library self;

	loader.set_import_resolver([self](const std::string& library, const std::string& function) -> void*
		{
			// dump mount point
			if (function == "XCgsqzn0000068")
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

	std::string data;
	if (!utils::io::read_file(binary, &data))
	{
		throw std::runtime_error
		{
			utils::string::va(
			"Failed to read game binary (%s)!\nPlease select the correct path in the launcher settings.",
			binary.data())
		};
	}

	auto mod = loader.load_library(binary);
	game::environment::set_module(mod);

	return mod.get_entry_point();
}

bool try_set_game_environment(launcher::game game)
{
	game::environment::set_game(game);

	try
	{
		auto install_path = game::environment::get_install_path();
		auto modules_path = install_path / (game == launcher::game::iidx ? "app" : "modules");

		SetCurrentDirectoryW(install_path.wstring().data());
		SetDllDirectoryW(modules_path.wstring().data());

		return true;
	}
	catch (std::exception&)
	{

	}

	return false;
}

int main()
{
	FARPROC entry_point;
	enable_dpi_awareness();

#if DEBUG
	create_console();
#endif

	std::srand(uint32_t(time(nullptr)));

	{
		auto premature_shutdown = true;
		const auto _ = gsl::finally([&premature_shutdown]
			{
				component_loader::pre_destroy();
			});

		try
		{
			const launcher launcher;
			if (!launcher.run())
				return 0;

			component_loader::create_components(game::environment::get_game());

			if (!component_loader::post_start())
				return 0;

			entry_point = load_binary(game::environment::get_game());

			if (!entry_point)
				throw std::runtime_error{ "Unable to load binary into memory" };

			if (!component_loader::post_load())
				return 0;
		}
		catch (std::exception& e)
		{
			MessageBoxA(nullptr, e.what(), "ERROR", MB_ICONERROR);
			return 1;
		}
	}

	return static_cast<int>(entry_point());
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	return main();
}
