#include <std_include.hpp>
#include "launcher/launcher.hpp"
#include "loader/component_loader.hpp"

#include <utils/nt.hpp>
#include <utils/hook.hpp>

#include <game/game.hpp>

void enable_dpi_awareness()
{
	const utils::nt::library user32{"user32.dll"};
	const auto set_dpi = user32
		                     ? user32.get_proc<BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT)>("SetProcessDpiAwarenessContext")
		                     : nullptr;
	if (set_dpi)
	{
		set_dpi(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	}
}

void create_console()
{
	AllocConsole();

	FILE* f;
	auto _ = freopen_s(&f, "CONOUT$", "w+t", stdout);
	_ = freopen_s(&f, "CONOUT$", "w", stderr);
	_ = freopen_s(&f, "CONIN$", "r", stdin);
}

int preinit()
{
	create_console();

	enable_dpi_awareness();
	std::srand(uint32_t(time(nullptr)));

	const launcher launcher;
	launcher.run();

	if (!component_loader::post_start())
		return 0;

	return 1;
}

int init()
{
	auto hr = game::avs_init();
	if (hr) return hr;

	if (!component_loader::post_load())
		return -1;

	return 0;
}

void cleanup()
{
	component_loader::pre_destroy();
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		launcher::dll_module = hModule;
		utils::hook::call(0x1401AFADC, preinit);
		utils::hook::call(0x1401F68ED, init);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		cleanup();
	}

	return true;
}