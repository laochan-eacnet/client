#include <std_include.hpp>
#include "launcher/launcher.hpp"
#include "loader/component_loader.hpp"

#include <utils/nt.hpp>
#include <utils/hook.hpp>
#include <utils/flags.hpp>

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

void cleanup()
{
	component_loader::pre_destroy();
}

void create_console()
{
	SetConsoleCP(932);
	std::locale::global(std::locale("ja_JP"));

	AllocConsole();

	FILE* f;
	auto _ = freopen_s(&f, "CONOUT$", "w+t", stdout);
	_ = freopen_s(&f, "CONOUT$", "w", stderr);
	_ = freopen_s(&f, "CONIN$", "r", stdin);

	SetConsoleCtrlHandler([](DWORD control_type) -> BOOL
	{
		if (control_type == CTRL_CLOSE_EVENT)
		{
			cleanup();
		}

		return true;
	}, true);
}

LRESULT WINAPI preinit(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	enable_dpi_awareness();
	std::srand(uint32_t(time(nullptr)));

	// fix launcher window not close after launcher, strange issue
	std::thread([]()
	{
		const launcher launcher;
		if (!launcher.run())
		{
			ExitProcess(0);
		}
	}).join();

	auto game_dir = game::install_dir() + "game";
	SetCurrentDirectoryA(game_dir.data());

	try
	{
		create_console();

		if (!component_loader::post_start())
			return 1;

		auto args = launcher::get_args();
		auto result = game::game_winmain(hInstance, hPrevInstance, args, nShowCmd);

		return result;
	}
	catch (std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "ERROR", MB_ICONERROR);
		return 1;
	}
}

int init()
{
	auto hr = game::init_avs();
	if (hr) return hr;

	try
	{
		if (!component_loader::post_load())
			return -1;
	}
	catch (std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "ERROR", MB_ICONERROR);
		return -1;
	}

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	if (utils::flags::has_flag("t"))
	{
		return true;
	}

	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		launcher::dll_module = hModule;

		utils::hook::call(0x1401FB4AD, preinit);
		utils::hook::call(0x140003B38, init);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		cleanup();
	}

	return true;
}
