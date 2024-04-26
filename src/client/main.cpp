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
	__try
	{
		component_loader::pre_destroy();
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ExitProcess(0);
	}
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
			cleanup();
		}

		return true;
	}, true);
}

int preinit(const char *args)
{
	if (utils::flags::has_flag("-t"))
		return game::game_preinit(args);

	try
	{
		if (!component_loader::pre_start())
			return 0;

		const launcher launcher;
		if (!launcher.run())
		{
			ExitProcess(0);
		}

#if DEBUG
		create_console();
#endif
		if (!component_loader::post_start())
			return 0;
	}
	catch (std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "ERROR", MB_ICONERROR);
		return 0;
	}

	return 1;
}

int init()
{
	auto hr = game::init_avs();
	if (hr) return hr;

	if (utils::flags::has_flag("-t"))
		return 0;

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
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		launcher::dll_module = hModule;
		utils::hook::call(0x1401BC3EC, preinit);
		utils::hook::call(0x14020333D, init);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		cleanup();
	}

	return true;
}
