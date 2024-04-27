#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <cstdlib>

#include <vector>
#include <string>

using namespace std::string_literals;

int __stdcall WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	int argc;
	auto * argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	std::vector<HANDLE> handles;

	for (size_t i = 0; i < argc; i++)
	{
		if (argv[i] == L"-event"s)
		{
			if (++i >= argc)
				break;

			auto handle = CreateEventW(NULL, true, false, argv[i]);

			if (handle)
			{
				handles.push_back(handle);
			}
		}
		else if (argv[i] == L"-process"s)
		{
			if (++i >= argc)
				break;

			auto handle = OpenProcess(SYNCHRONIZE, false, _wtoi(argv[i]));

			if (handle)
			{
				handles.push_back(handle);
			}
		}
	}

	WaitForMultipleObjects(static_cast<DWORD>(handles.size()), handles.data(), false, INFINITE);

	for (auto& handle : handles)
	{
		CloseHandle(handle);
	}

	return S_OK;
}
