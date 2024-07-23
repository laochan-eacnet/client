#pragma once

#define WEAK __declspec(selectany)
#include "loader/component_loader.hpp"

namespace game
{
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
			const auto new_capacity = (new_size + 1) > 0x10 ? (new_size + 1) : 0x10;
			auto buffer = reinterpret_cast<char*>(std::malloc(new_capacity));

			std::memcpy(buffer, this->data(), this->size());
			std::memcpy(buffer + this->size(), str.data(), str.size());
			buffer[new_size] = 0;

			if (this->capacity() >= 0x10)
				std::free(this->data());

			this->capacity_ = new_capacity;
			this->size_ = new_size;
			this->data_.ptr = buffer;
		}
	};
}

namespace avs2
{
	WEAK function<file_t(const char* name, uint16_t mode, int flags)> fs_open{ "XCgsqzn000004e" };
	WEAK function<int(const char* sname, const char* dname)> fs_copy{ "XCgsqzn0000065" };
	WEAK function<void(file_t file)> fs_close{ "XCgsqzn0000055" };
	WEAK function<int(file_t file, struct stat* stat)> fs_fstat{ "XCgsqzn0000062" };
	WEAK function<int(const char* path, struct stat* stat)> fs_lstat{ "XCgsqzn0000063" };
	WEAK function<int(file_t file, long offset, int origin)> fs_lseek{ "XCgsqzn000004f" };
	WEAK function<size_t(file_t file, const char* data, uint32_t data_size)> fs_read{ "XCgsqzn0000051" };
	WEAK function<file_t(const char* path)> fs_opendir{ "XCgsqzn000005c" };
	WEAK function<const char* (file_t dir)> fs_readdir{ "XCgsqzn000005d" };
	WEAK function<void(file_t dir)> fs_closedir{ "XCgsqzn000005e" };
	WEAK function<int(const char* mountpoint, const char* fsroot, const char* fstype, void* data)> fs_mount{ "XCgsqzn000004b" };
	WEAK function<int(node_ptr, void*, size_t, void*, void*, HANDLE)> boot{ "XCgsqzn0000129" };
	WEAK function<int()> fs_dump_mountpoint{ "XCgsqzn0000068" };

	WEAK function<node_ptr(property_ptr prop, node_ptr node, node_type type, const char* path, ...)> property_node_create{ "XCgsqzn00000a2" };
	WEAK function<node_ptr(property_ptr prop, node_ptr node, const char* path)> property_search{ "XCgsqzn00000a2" };
	WEAK function<property_ptr(node_ptr node)> property_node_get_desc{ "XCgsqzn00000ae" };
	WEAK function<void(node_ptr node)> property_remove{ "XCgsqzn00000a3" };
	WEAK function<int(property_ptr prop, uint8_t* data, uint32_t data_size)> property_mem_write{ "XCgsqzn00000b8" };
	WEAK function<int(property_ptr prop, node_ptr node, node_stat_ptr stat)> property_node_query_stat{ "XCgsqzn00000c5" };
	WEAK function<int(property_ptr prop, uint32_t set_flags, uint32_t clear_flags)> property_set_flag{ "XCgsqzn000009a" };
}
