#pragma once

#include <string>

namespace utils
{
	class image
	{
	public:
		image(const std::string& data);

		uint32_t get_width() const;
		uint32_t get_height() const;
		const void* get_buffer() const;
		size_t get_size() const;

		const std::string& get_data() const;

	private:
		uint32_t width{};
		uint32_t height{};
		std::string data{};
	};
}
