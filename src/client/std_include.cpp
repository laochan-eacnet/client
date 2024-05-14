#include <std_include.hpp>

#pragma comment(linker, "/base:0x1400000")

extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 1;
	__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 1;

	HRESULT WINAPI DirectInput8Create_L(
		HINSTANCE hinst, DWORD dwVersion, const IID* const riidltf, LPVOID* ppvOut, LPVOID* punkOuter
	)
	{
		static auto func = [] {
			char dir[MAX_PATH]{ 0 };
			GetSystemDirectoryA(dir, sizeof(dir));
			strcat_s(dir, "/dinput8.dll");

			const auto dinput8 = LoadLibraryA(dir);
			return reinterpret_cast<decltype(&DirectInput8Create_L)>(GetProcAddress(dinput8, "DirectInput8Create"));
			}();

			return func(hinst, dwVersion, riidltf, ppvOut, punkOuter);
	}

	__declspec(dllexport) __declspec(naked) void DirectInput8Create() 
	{
		__asm jmp DirectInput8Create_L
	}
};

extern "C"
{
	int s_read_arc4random(void*, size_t)
	{
		return -1;
	}

	int s_read_getrandom(void*, size_t)
	{
		return -1;
	}

	int s_read_urandom(void*, size_t)
	{
		return -1;
	}

	int s_read_ltm_rng(void*, size_t)
	{
		return -1;
	}
}
