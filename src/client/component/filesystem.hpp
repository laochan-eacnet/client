#pragma once

namespace filesystem
{
	class file
	{
	public:
		file(std::string name);

		[[nodiscard]] bool exists() const;
		[[nodiscard]] const std::string& get_buffer() const;
		[[nodiscard]] const std::string& get_name() const;

	private:
		bool valid_ = false;
		std::string name_;
		std::string buffer_;
	};

	bool exists(const std::string& path);
}
