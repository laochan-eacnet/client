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

	WEAK symbol<bool(void*)> dan_select_flow_attach{ 0x14011F4D0 };
	WEAK symbol<void(void*)> dan_select_flow_detach{ 0x14011F500 };

	WEAK symbol<void(StageResultDrawFrame_s*, void*)> stage_result_draw_frame_init{ 0x140175E10 };

	namespace EacnetRequestPost
	{
		WEAK symbol<bool(eacnet_request_post_s*, void*, void*)> OnRequestPropertyExported{ 0x140302CD0 };
	}

	WEAK symbol<void*(size_t size)> malloc{ 0x1403DEA10 };
	WEAK symbol<void(void* block)> free{ 0x1403DE9D0 };

	WEAK symbol<IDirect3DDevice9Ex*> d3d9ex_device{ 0x142AAC410 };
	WEAK symbol<IDirect3DDevice9*>  d3d9_device{ 0x142AAC410 };
	WEAK symbol<HWND*> main_hwnd{ 0x142AAC400 };
	WEAK symbol<const char> infinitas_id{ 0x141AC4494 };
	WEAK symbol<const char> game_version{ 0x141AC44E3 };

	WEAK symbol<bool> show_options{ 0x141ACD104 };
	WEAK symbol<bool> show_consume_window{ 0x1405EE970 };
	WEAK symbol<int> selected_gauge_type{ 0x141ACD138 };

	WEAK symbol<state_t> state{ 0x1426A05C4 };

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
	WEAK avs_function<int(node_ptr, void*, size_t, void*, void*, HANDLE)> avs_boot{ "XCgsqzn0000129" };

	WEAK avs_function<node_ptr(property_ptr prop, node_ptr node, node_type type, const char* path, ...)> property_node_create{ "XCgsqzn00000a2" };
	WEAK avs_function<node_ptr(property_ptr prop, node_ptr node, const char* path)> property_search{ "XCgsqzn00000a2" };
	WEAK avs_function<property_ptr(node_ptr node)> property_node_getprop{ "XCgsqzn00000ae" };
	WEAK avs_function<void(node_ptr node)> property_remove{ "XCgsqzn00000a3" };
	WEAK avs_function<int(property_ptr prop, uint8_t* data, uint32_t data_size)> property_mem_write{ "XCgsqzn00000b8" };
	WEAK avs_function<int(property_ptr prop, node_ptr node, node_stat_ptr stat)> property_node_query_stat{ "XCgsqzn00000c5" };
	WEAK avs_function<int(property_ptr prop, uint32_t set_flags, uint32_t clear_flags)> property_set_flag{ "XCgsqzn000009a" };

	class string final
	{
	private:
		union
		{
			char* ptr;
			char raw[16];
		} data_;

		size_t size_;
		size_t capacity_;

	public:
		inline size_t size() { return this->size_; }
		inline void clear() { this->size_ = 0; }
		inline size_t capacity() { return this->capacity_; };
		inline char* data()
		{
			if (this->capacity() >= 0x10)
				return this->data_.ptr;

			return this->data_.raw;
		}

		inline void append(const std::string& str)
		{
			const auto new_size = str.size() + this->size_;
			const auto new_capacity = new_size > 0x10 ? new_size : 0x10;
			auto buffer = reinterpret_cast<char*>(game::malloc(new_capacity));

			std::memcpy(buffer, this->data(), this->size());
			std::memcpy(buffer + this->size(), str.data(), str.size());

			if (this->capacity() >= 0x10)
				game::free(this->data());

			this->capacity_ = new_capacity;
			this->size_ = new_size;
			this->data_.ptr = buffer;
		}
	};
}
