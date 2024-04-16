#pragma once

#define WEAK __declspec(selectany)

namespace game
{
	WEAK symbol<bool(const char* args)> game_preinit{ 0x1401AF440 };
	WEAK symbol<bool()> init_avs{ 0x1400BA240 };
	WEAK symbol<void()> finalize_music_data{ 0x1400CD710 };
	WEAK symbol<music_data_t* ()> get_music_data{ 0x1401B76B0 };
	WEAK symbol<LRESULT(HWND, UINT, WPARAM, LPARAM)> main_wndproc{ 0x1401F5550 };

	WEAK symbol<IDirect3DDevice9Ex*> d3d9ex_device{ 0x142A9C5B8 };
	WEAK symbol<IDirect3DDevice9*>  d3d9_device{ 0x142A9C5B8 };
	WEAK symbol<HWND*> main_hwnd{ 0x142A9C5A8 };
	WEAK symbol<const char> infinitas_id { 0x141AB4494 };
	WEAK symbol<const char> game_version { 0x141AB44E3 };

	typedef uint32_t avs_file_t;

	WEAK avs_function<avs_file_t(const char* name, uint16_t mode, int flags)> avs_fs_open{ "XCgsqzn000004e" };
	WEAK avs_function<int(const char* sname, const char* dname)> avs_fs_copy{ "XCgsqzn0000065" };
	WEAK avs_function<void(avs_file_t file)> avs_fs_close{ "XCgsqzn0000055" };
	WEAK avs_function<int(avs_file_t file, struct avs_stat* stat)> avs_fs_fstat{ "XCgsqzn0000062" };
	WEAK avs_function<int(const char* path, struct avs_stat* stat)> avs_fs_lstat{ "XCgsqzn0000063" };
	WEAK avs_function<int(avs_file_t file, long offset, int origin)> avs_fs_lseek{ "XCgsqzn000004f" };
	WEAK avs_function<size_t(avs_file_t file, const char* data, uint32_t data_size)> avs_fs_read{ "XCgsqzn0000051" };
	WEAK avs_function<avs_file_t(const char* path)> avs_fs_opendir{ "XCgsqzn000005c" };
	WEAK avs_function<const char* (avs_file_t dir)> avs_fs_readdir{ "XCgsqzn000005d" };
	WEAK avs_function<void(avs_file_t dir)> avs_fs_closedir{ "XCgsqzn000005e" };
}
