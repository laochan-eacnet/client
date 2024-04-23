#include "image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <gsl/gsl>

namespace utils
{
	image::image(const std::string& image_data)
	{
		int channels{};

		int w, h;

		auto* rgb_image = stbi_load_from_memory(reinterpret_cast<const uint8_t*>(image_data.data()), static_cast<int>(image_data.size()), &w, &h, &channels, 4);
		if(!rgb_image)
		{
			throw std::runtime_error("Unable to load image");
		}

		this->width = static_cast<uint32_t>(w);
		this->height = static_cast<uint32_t>(h);

		auto _ = gsl::finally([rgb_image]()
		{
			stbi_image_free(rgb_image);
		});

		const auto size = this->width * this->height * 4;
		this->data.resize(size);
		
		std::memmove(this->data.data(), rgb_image, size);
	}

	uint32_t image::get_width() const
	{
		return this->width;
	}

	uint32_t image::get_height() const
	{
		return this->height;
	}

	const void* image::get_buffer() const
	{
		return this->data.data();
	}

	size_t image::get_size() const
	{
		return this->data.size();
	}

	const std::string& image::get_data() const
	{
		return this->data;
	}
}
