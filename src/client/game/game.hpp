#pragma once

#include "launcher/launcher.hpp"
#include <utils/nt.hpp>

namespace game
{
	template <typename T>
	class symbol
	{
	public:
		symbol(const size_t address): object_(reinterpret_cast<T*>(address))
		{
		}

		T* get() const
		{
			return object_;
		}

		operator T*() const
		{
			return this->get();
		}

		T* operator->() const
		{
			return this->get();
		}

	private:
		T* object_;
	};

	template <typename T>
	class avs_function
	{
	public:
		avs_function(const char *name)
		{
			utils::nt::library avs_core{ "avs2-core.dll" };
			this->object_ = reinterpret_cast<T*>(avs_core.get_proc<void*>(name));
		}

		T* get() const
		{
			return object_;
		}

		operator T* () const
		{
			return this->get();
		}

		T* operator->() const
		{
			return this->get();
		}

	private:
		T* object_;
	};

	template<typename T>
	struct basic_string_vc10
	{
		union
		{
			T* ptr; // size is > 0x10
			T data[1];

			uint8_t raw[16];
		};
		size_t size;
		size_t capacity;
	};

	template<typename T>
	static inline T* basic_string_vc10_data(basic_string_vc10<T>* str)
	{
		if (sizeof(T) * str->size >= 0x10)
			return str->ptr;

		return str->data;
	}

	typedef basic_string_vc10<char> string;

	inline std::string install_dir()
	{
		HKEY key;

		if (RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\KONAMI\\GITADORA", 0, KEY_READ | KEY_WOW64_64KEY, &key)
			&& RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\KONAMI\\GITADORA", 0, KEY_READ | KEY_WOW64_64KEY, &key))
		{
			throw std::exception("Game is not installed properly (key is not found)");
		}

		DWORD type, size;

		if (RegQueryValueExA(key, "InstallDir", 0, &type, 0, &size))
		{
			throw std::exception("Game is not installed properly (failed to get key value size)");
		}

		if (type != 1)
		{
			throw std::exception("Game is not installed properly (illegal key value type)");
		}

		std::string path;
		path.resize(size - 1);

		if (RegQueryValueExA(key, "InstallDir", 0, &type, reinterpret_cast<LPBYTE>(path.data()), &size))
		{
			RegCloseKey(key);
			throw std::exception("failed to read key value");
		}

		RegCloseKey(key);

		return path;
	}
}

#include "struct.hpp"
#include "symbols.hpp"
