#pragma once

#include "launcher/launcher.hpp"
#include "loader/component_loader.hpp"
#include <utils/nt.hpp>
#include <utils/signature.hpp>
#include <utils/string.hpp>

#define IIDX_TARGET_VERSION "P2D:J:B:A:2024080500"
#define SDVX_TARGET_VERSION "ANY"
#define GITADORA_TARGET_VERSION "ANY"
#define DDR_TARGET_VERSION "ANY"

namespace game
{
	namespace environment
	{
		launcher::game get_game();

		bool is_iidx();
		bool is_sdvx();
		bool is_gitadora();
		bool is_ddr();

		void set_game(launcher::game mode);

		std::string get_string();
		std::string get_string(launcher::game);
		std::wstring get_stringw();
		std::wstring get_stringw(launcher::game);
		std::string get_version();
		utils::nt::library& get_module();
		void set_module(utils::nt::library&);

		std::string get_param(std::string const& key);
		void set_param(std::string const& key, std::string const& value);

		std::filesystem::path get_install_path();
		std::filesystem::path get_resource_path();
		static std::filesystem::path get_install_path(launcher::game);
		static std::filesystem::path get_resource_path(launcher::game);

		class gamemeta
		{
		public:
			bool get_install_state();

			std::wstring get_install_path();
			std::wstring get_resource_path();
			std::wstring get_game_module_path();
			std::wstring get_settings_module_path();
			std::wstring get_updater_module_path();

			std::string get_game_module_name();
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
			std::string game_module_name;
			std::string game_module_version;
			std::string game_module_target_version;
			void ensure_created(launcher::game);
		};
	}

	template <typename T>
	class vftable_entry : resolve_after_load_symbol_interface
	{
	public:
		vftable_entry(const size_t vftable, size_t n, launcher::game game) : vftable_(vftable), n_(n), object_(nullptr)
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
		symbol(const size_t address) : object_(reinterpret_cast<T*>(address))
		{
		}

		T* get() const
		{
			[[unlikely]]
			if (!object_)
				throw std::runtime_error{ __FUNCSIG__ ": unresolved" };

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

	protected:
		T* object_;
	};

	template <typename T>
	class pattern : public symbol<T>, resolve_after_load_symbol_interface
	{
	public:
		pattern(const std::string pattern, 
				int32_t offset, 
				launcher::game game, 
				size_t n = 0) : symbol<T>(0)
		{
			pattern_ = pattern;
			offset_ = offset;
			n_ = n;
			::component_loader::register_symbol(this, game);
		}

		void resolve()
		{
			const auto& game_module = environment::get_module();

			if (!n_)
			{
				auto result = game_module.match_sig(this->pattern_);

				[[unlikely]]
				if (!result)
					throw std::runtime_error{ __FUNCSIG__ ": failed to resolve symbol." };

				this->object_ = reinterpret_cast<T*>(result + this->offset_);
			}
			else
			{
				auto results = game_module.match_sigs(this->pattern_);

				[[unlikely]]
				if (results.size() < n_)
					throw std::runtime_error{ __FUNCSIG__ ": failed to resolve symbol." };

				this->object_ = reinterpret_cast<T*>(results[n_] + this->offset_);
			}

			printf("%s: %p\n", __FUNCSIG__, this->object_);
		}

	protected:
		std::string pattern_;
		size_t offset_;
		size_t n_;
	};

	template <typename T>
	class pattern_extract : public pattern<T>
	{
	public:
		pattern_extract(const std::string pattern, 
						int32_t offset, 
						launcher::game game, 
						size_t n = 0) : pattern<T>(pattern, offset, game, n) {
			rva_pos_ = nullptr;
		}

		void resolve()
		{
			pattern<T>::resolve();
	
			this->rva_pos_ = reinterpret_cast<uint8_t*>(this->object_);
			const auto offset = *reinterpret_cast<int32_t*>(this->rva_pos_);
			this->object_ = reinterpret_cast<T*>(this->rva_pos_ + offset + 4);

			printf("%s: %p\n", __FUNCSIG__, this->object_);
		}

		uint8_t* get_rva_pos()
		{
			return rva_pos_;
		}

	public:
		uint8_t *rva_pos_;
	};

	class pattern_extract_vftable : public pattern_extract<size_t>
	{
	public:
		pattern_extract_vftable(const std::string pattern,
						int32_t offset,
						launcher::game game,
						size_t n = 0) : pattern_extract<size_t>(pattern, offset, game, n) {}

		void* get_ptr(size_t index) const
		{
			return reinterpret_cast<void*>(this->object_ + index);
		}

		template <typename T>
		T* get(size_t index) const
		{
			return *reinterpret_cast<T**>(this->get_ptr(index));
		}

		template <typename T, typename... Args>
		static T invoke(size_t index, Args ... args)
		{
			return this->get<T(*)(Args ...)>(index)(args...);
		}

		size_t* operator->() const = delete;
	};
}

namespace avs2
{
	template <typename T>
	class function : resolve_after_load_symbol_interface
	{
	public:
		function(const char* name) : name_(name), object_(nullptr)
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
