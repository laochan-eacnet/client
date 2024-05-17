#pragma once

#define WEAK __declspec(selectany)

namespace game
{
	WEAK symbol<int __fastcall(const char*, const char* selector_name)> AvsBootCall{ 0x5A3C00 };
	WEAK symbol<void()> DllInitCall{ 0x5A3E20 };
	WEAK symbol<int()> DllMainCall{ 0x5A3EA0 };
	WEAK symbol<void()> LocalstrapShutdown{ 0x5A3EB0 };
	WEAK symbol<APPERROR __fastcall(const char* extdrmfs)> arg_adj_extfs{ 0x5AC750 };

	typedef uint32_t avs_file_t;

	WEAK avs_function<avs_file_t(const char* name, uint16_t mode, int flags)> avs_fs_open{ "XCd229cc000090" };
	WEAK avs_function<int(const char* sname, const char* dname)> avs_fs_copy{ "XCd229cc0000eb" };
	WEAK avs_function<void(avs_file_t file)> avs_fs_close{ "XCd229cc00011f" };
	WEAK avs_function<int(avs_file_t file, struct avs_stat* stat)> avs_fs_fstat{ "XCd229cc0000c3" };
	WEAK avs_function<int(const char* path, struct avs_stat* stat)> avs_fs_lstat{ "XCd229cc0000c0" };
	WEAK avs_function<int(avs_file_t file, long offset, int origin)> avs_fs_lseek{ "XCd229cc00004d" };
	WEAK avs_function<size_t(avs_file_t file, const char* data, uint32_t data_size)> avs_fs_read{ "XCd229cc00010d" };
	WEAK avs_function<avs_file_t(const char* path)> avs_fs_opendir{ "XCd229cc0000f0" };
	WEAK avs_function<const char* (avs_file_t dir)> avs_fs_readdir{ "XCd229cc0000bb" };
	WEAK avs_function<void(avs_file_t dir)> avs_fs_closedir{ "XCd229cc0000b8" };
	WEAK avs_function<size_t(const char* mountpoint, const char* fsroot, const char* fstype, void* data)> avs_fs_mount{ "XCd229cc0000ce" };
}
