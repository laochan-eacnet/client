#pragma once

#include <string>
#include <unordered_map>

#define CHUNK 16384u

namespace utils::compression
{
	namespace zstd
	{
		std::string compress(const std::string& data);
		std::string decompress(const std::string& data);
	}
};
