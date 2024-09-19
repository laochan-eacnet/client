#include "string.hpp"
#include <sstream>
#include <cstdarg>
#include <algorithm>

#include "nt.hpp"

namespace utils::string
{
	const char* va(const char* fmt, ...)
	{
		static va_provider<8, 256> provider;

		va_list ap;
		va_start(ap, fmt);

		const char* result = provider.get(fmt, ap);

		va_end(ap);
		return result;
	}

	std::vector<std::string> split(const std::string& s, const char delim)
	{
		std::stringstream ss(s);
		std::string item;
		std::vector<std::string> elems;

		while (std::getline(ss, item, delim))
		{
			elems.push_back(item); // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
		}

		return elems;
	}

	std::string to_lower(const std::string& text)
	{
		std::string result;
		std::ranges::transform(text, std::back_inserter(result), [](const unsigned char input)
		{
			return static_cast<char>(std::tolower(input));
		});

		return result;
	}

	std::string to_upper(const std::string& text)
	{
		std::string result;
		std::ranges::transform(text, std::back_inserter(result), [](const unsigned char input)
		{
			return static_cast<char>(std::toupper(input));
		});

		return result;
	}

	bool starts_with(const std::string& text, const std::string& substring)
	{
		return text.find(substring) == 0;
	}

	bool ends_with(const std::string& text, const std::string& substring)
	{
		if (substring.size() > text.size()) return false;
		return std::equal(substring.rbegin(), substring.rend(), text.rbegin());
	}

	std::string dump_hex(const std::string& data, const std::string& separator)
	{
		std::string result;

		for (unsigned int i = 0; i < data.size(); ++i)
		{
			if (i > 0)
			{
				result.append(separator);
			}

			result.append(va("%02x", data[i] & 0xFF));
		}

		return result;
	}

	std::string get_clipboard_data()
	{
		if (OpenClipboard(nullptr))
		{
			std::string data;

			auto* const clipboard_data = GetClipboardData(1u);
			if (clipboard_data)
			{
				auto* const cliptext = static_cast<char*>(GlobalLock(clipboard_data));
				if (cliptext)
				{
					data.append(cliptext);
					GlobalUnlock(clipboard_data);
				}
			}
			CloseClipboard();

			return data;
		}
		return {};
	}

	void strip(const char* in, char* out, size_t max)
	{
		if (!in || !out) return;

		max--;
		auto current = 0u;
		while (*in != 0 && current < max)
		{
			const auto color_index = (*(in + 1) - 48) >= 0xC ? 7 : (*(in + 1) - 48);

			if (*in == '^' && (color_index != 7 || *(in + 1) == '7'))
			{
				++in;
			}
			else
			{
				*out = *in;
				++out;
				++current;
			}

			++in;
		}

		*out = '\0';
	}

	std::string convert(const std::wstring& wstr)
	{
		std::string result;
		result.reserve(wstr.size());

		for (const auto& chr : wstr)
		{
			result.push_back(static_cast<char>(chr));
		}

		return result;
	}

	std::wstring convert(const std::string& str)
	{
		std::wstring result;
		result.reserve(str.size());

		for (const auto& chr : str)
		{
			result.push_back(static_cast<wchar_t>(chr));
		}

		return result;
	}

	std::string replace(std::string str, const std::string& from, const std::string& to)
	{
		if (from.empty())
		{
			return str;
		}

		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos)
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}

		return str;
	}

	std::wstring shiftjis_to_wide(std::string str)
	{
		std::wstring buffer;

		auto size = MultiByteToWideChar(932, 0, str.data(), -1, nullptr, 0);
		size--;
		buffer.resize(size);

		MultiByteToWideChar(932, 0, str.data(), -1, buffer.data(), size);

		return buffer;
	}

	std::wstring utf8_to_wide(std::string str)
	{
		std::wstring buffer;

		auto size = MultiByteToWideChar(65001, 0, str.data(), -1, nullptr, 0);
		size--;
		buffer.resize(size);

		MultiByteToWideChar(65001, 0, str.data(), -1, buffer.data(), size);

		return buffer;
	}

	std::string wide_to_shiftjis(std::wstring str)
	{
		std::string buffer;

		auto size = WideCharToMultiByte(932, 0, str.data(), -1, nullptr, 0, nullptr, nullptr);
		size--;
		buffer.resize(size);

		WideCharToMultiByte(932, 0, str.data(), -1, buffer.data(), size, nullptr, nullptr);

		return buffer;
	}

	std::string wide_to_utf8(std::wstring str)
	{
		std::string buffer;

		auto size = WideCharToMultiByte(65001, 0, str.data(), -1, nullptr, 0, nullptr, nullptr);//这里返回的大小包含null
		size--;
		buffer.resize(size);

		WideCharToMultiByte(65001, 0, str.data(), -1, buffer.data(), size, nullptr, nullptr);//如果显式指定大小则可以不包含null

		return buffer;
	}
}
