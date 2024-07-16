#pragma once

#include "launcher/launcher.hpp"
#include <utils/nt.hpp>

namespace game
{
	namespace environment
	{
		launcher::game get_game();

		bool is_iidx();
		bool is_sdvx();
		bool is_gitadora();

		void set_game(launcher::game mode);

		std::string get_string();
		std::string get_version();
		utils::nt::library& get_module();
		void set_module(utils::nt::library&);

		std::filesystem::path get_install_path();
		std::filesystem::path get_resource_path();
	}

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

namespace avs2
{
	template <typename T>
	class function
	{
	public:
		function(const char* name)
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
}

#include "struct.hpp"
#include "avs2.hpp"
#include "symbols.hpp"
