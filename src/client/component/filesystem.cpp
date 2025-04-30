#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "game/game.hpp"

#include "filesystem.hpp"

#include <utils/flags.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>

// from ifs_layeredfs
extern "C" int init(const char* data_dir);

namespace filesystem
{
	file::file(std::string name)
		: name_(name)
	{
		avs2::stat stat = { 0 };
		if (!avs2::fs_lstat(name.data(), &stat) || stat.filesize <= 0)
		{
			this->valid_ = false;
			return;
		}

		auto file = avs2::fs_open(name_.data(), 1, 420);

		if (file <= 0)
		{
			this->valid_ = false;
			return;
		}

		avs2::fs_fstat(file, &stat);

		if (stat.filesize <= 0)
		{
			this->valid_ = false;
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
		if (!avs2::fs_lstat(path.data(), &stat) || stat.filesize <= 0)
			return false;

		return true;
	}

	class component final : public component_interface
	{
	public:
		void post_avs_init() override
		{
			utils::nt::library self{ };

			auto g = game::environment::get_game();
			std::filesystem::path data_dir = self.get_folder();
			data_dir /= "assets";

			if (g == launcher::game::iidx)
				data_dir /= "iidx";
			else if (g == launcher::game::sdvx)
				data_dir /= "sdvx";
			else if (g == launcher::game::gitadora)
				data_dir /= "gitadora";

			static auto abs_data_path = std::filesystem::absolute(data_dir).generic_string();

			if (!std::filesystem::exists(data_dir))
				printf("warning: data dir %s not exists\n", abs_data_path.data());

			if (!utils::flags::has_flag("disable_ifs_hook"))
				init(std::filesystem::exists(data_dir) ? abs_data_path.data() : nullptr);

			avs2::fs_mount("/laochan", abs_data_path.data(), "fs", nullptr);
		}
	};
}

REGISTER_COMPONENT(filesystem::component, launcher::game::all)
