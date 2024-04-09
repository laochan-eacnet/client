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
}

#include "symbols.hpp"
