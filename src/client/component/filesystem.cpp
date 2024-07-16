#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include "filesystem.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

namespace filesystem
{
	file::file(std::string name)
		: name_(std::move(name))
	{
		auto file = avs2::fs_open(name_.data(), 1, 420);

		avs2::stat stat = { 0 };
		avs2::fs_fstat(file, &stat);

		if (stat.filesize <= 0)
		{
			avs2::fs_close(file);
			return;
		}

		auto* buffer = utils::memory::allocate<char>(stat.filesize);
		const auto size = avs2::fs_read(file, buffer, stat.filesize);

		auto _ = gsl::finally([=] {
			utils::memory::free(buffer);
			avs2::fs_close(file);
		});

		if (size >= 0)
		{
			this->valid_ = true;
			this->buffer_.append(buffer, size);
		}
	}

	bool file::exists() const
	{
		return this->valid_;
	}

	const std::string& file::get_buffer() const
	{
		return this->buffer_;
	}

	const std::string& file::get_name() const
	{
		return this->name_;
	}

	bool exists(const std::string& path)
	{
		avs2::stat stat = { 0 };
		avs2::fs_lstat(path.data(), &stat);

		if (stat.filesize > 0)
			return true;

		return false;
	}
}
