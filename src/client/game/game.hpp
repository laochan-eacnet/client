#pragma once

#include "launcher/launcher.hpp"

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
}

#include "struct.hpp"
#include "symbols.hpp"
