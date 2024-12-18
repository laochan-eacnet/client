#include "memory.hpp"
#include "compression.hpp"

#include <zstd.h>

namespace utils::compression
{
	namespace zstd
	{
		std::string compress(const std::string& data)
		{
			auto size = ZSTD_compressBound(data.size());
			std::string buffer{};
			buffer.resize(static_cast<size_t>(size));

			auto compressed_size = ZSTD_compress(buffer.data(), buffer.size(), data.data(), data.size(), ZSTD_maxCLevel());

			if (ZSTD_isError(compressed_size)) return {};

			buffer.resize(compressed_size);
			return buffer;
		}

		std::string decompress(const std::string& data)
		{
			auto size = ZSTD_getFrameContentSize(data.data(), data.size());
			std::string buffer{};
			buffer.resize(static_cast<size_t>(size));

			auto decompressed_size = ZSTD_decompress(buffer.data(), buffer.size(), data.data(), data.size());

			if (ZSTD_isError(decompressed_size)) return {};

			buffer.resize(decompressed_size);
			return buffer;
		}
	}
}
