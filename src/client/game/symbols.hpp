#pragma once

#define WEAK __declspec(selectany)

namespace game
{
	WEAK symbol<bool(const char* args)> game_preinit{ 0x1401BBD50 };
	WEAK symbol<bool()> init_avs{ 0x1400C6410 };
	WEAK symbol<void()> finalize_music_data{ 0x1400D98E0 };
	WEAK symbol<music_data_t* ()> get_music_data{ 0x1401C3FC0 };
	WEAK symbol<LRESULT(HWND, UINT, WPARAM, LPARAM)> main_wndproc{ 0x140201FA0 };

	WEAK symbol<bool(void*)> music_select_scene_attach{ 0x140144220 };
	WEAK symbol<void(void*)> music_select_scene_detach{ 0x140144780 };

	WEAK symbol<IDirect3DDevice9Ex*> d3d9ex_device{ 0x142AAC410 };
	WEAK symbol<IDirect3DDevice9*>  d3d9_device{ 0x142AAC410 };
	WEAK symbol<HWND*> main_hwnd{ 0x142AAC400 };
	WEAK symbol<const char> infinitas_id{ 0x141AC4494 };
	WEAK symbol<const char> game_version{ 0x141AC44E3 };

	WEAK symbol<bool> show_options{ 0x141ACD104 };
	WEAK symbol<bool> selected_gauge_type{ 0x141ACD138 };

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
	WEAK avs_function<int(const char* mountpoint, const char* fsroot, const char* fstype, void* data)> avs_fs_mount{ "XCgsqzn000004b" };
}
