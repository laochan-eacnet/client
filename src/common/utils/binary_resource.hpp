#pragma once

#include <string>
#include <Windows.h>

namespace utils
{
	class binary_resource
	{
	public:
		binary_resource(HMODULE module, int id, std::string file);

		std::string get_extracted_file(bool fatal_if_overwrite_fails = false);
		const std::string& get_data() const;

	private:
		std::string resource_;
		std::string filename_;
		std::string path_;
	};
}
