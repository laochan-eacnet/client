#include <std_include.hpp>
#include "launcher/launcher.hpp"
#include "loader/component_loader.hpp"

#include <utils/nt.hpp>
#include <utils/hook.hpp>
#include <utils/memory.hpp>

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

int win_main(HINSTANCE hinst, HINSTANCE hinstPrev, char* lpszCmdLine, int nCmdShow)
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

	create_console();

	try
	{
		auto game_dir = game::install_dir() + "game";
		SetCurrentDirectoryA(game_dir.data());

		if (!component_loader::post_start())
			return -2;

		game::AvsBootCall("_", "_");

		if (!component_loader::post_load())
			return -3;

		game::DllInitCall();
		auto retcode = game::DllMainCall();
		game::LocalstrapShutdown();

		return retcode;
	}
	catch (std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "ERROR", MB_ICONERROR);
		return -1;
	}

	return -1;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		launcher::dll_module = hModule;
		utils::hook::jump(0x5A40C0, win_main);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		cleanup();
	}

	return true;
}
