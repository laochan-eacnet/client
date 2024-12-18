#pragma once

#include "launcher/launcher.hpp"
#include "loader/component_loader.hpp"
#include <utils/nt.hpp>
#include <utils/string.hpp>

#define IIDX_TARGET_VERSION "P2D:J:B:A:2024080500"
#define SDVX_TARGET_VERSION "ANY"
#define GITADORA_TARGET_VERSION "U32:J:A:A:2024021300"

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
		std::string get_string(launcher::game);
		std::wstring get_stringw();
		std::wstring get_stringw(launcher::game);
		std::string get_version();
		utils::nt::library& get_module();
		void set_module(utils::nt::library&);

		std::string get_param(std::string const &key);
		void set_param(std::string const &key, std::string const &value);

		std::filesystem::path get_install_path();
		std::filesystem::path get_resource_path();
		std::filesystem::path get_install_path(launcher::game);
		std::filesystem::path get_resource_path(launcher::game);


		class gamemeta
		{
		public:
			bool get_install_state();

			std::wstring get_install_path();
			std::wstring get_resource_path();
			std::wstring get_game_module_path();
			std::wstring get_settings_module_path();
			std::wstring get_updater_module_path();

			std::string get_game_module_version();
			std::string get_game_module_target_version();

			static gamemeta get_gamemeta(launcher::game);
		private:
			launcher::game game_type = launcher::game::invalid;
			bool installed = false;
			bool inited = false;
			std::filesystem::path install_path;
			std::filesystem::path resource_path;
			std::filesystem::path game_module_path;
			std::filesystem::path settings_module_path;
			std::filesystem::path updater_module_path;
			std::string game_module_version;
			std::string game_module_target_version;
			void ensure_created(launcher::game);
		};
	}

	template <typename T>
	class vftable_entry: resolve_after_load_symbol_interface
	{
	public:
		vftable_entry(const size_t vftable, size_t n, launcher::game game): vftable_(vftable), n_(n), object_(nullptr)
		{
			::component_loader::register_symbol(this, game);
		}

		void resolve()
		{
			object_ = reinterpret_cast<T*>(reinterpret_cast<size_t*>(vftable_)[n_]);
			printf("resolve %s to %p\n", __FUNCSIG__, this->object_);
		}

		size_t get_entry_ptr() const
		{
			return vftable_ + n_ * sizeof(size_t);
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
		size_t vftable_;
		size_t n_;
	};

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
	class function : resolve_after_load_symbol_interface
	{
	public:
		function(const char* name): name_(name), object_(nullptr)
		{
			::component_loader::register_symbol(this, launcher::game::all);
		}

		void resolve()
		{
			utils::nt::library avs_core{ "avs2-core.dll" };
			this->object_ = reinterpret_cast<T*>(avs_core.get_proc<void*>(this->name_));

			if (!this->object_)
			{
				throw std::runtime_error(utils::string::va("failed to find %s of avs2\n", this->name_));
			}

			printf("%s: %p\n", __FUNCSIG__, this->object_);
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
		const char* name_;
	};
}

#include "avs2.hpp"
#include "struct.hpp"
#include "symbols.hpp"

#include "iidx/struct.hpp"
#include "iidx/symbols.hpp"
