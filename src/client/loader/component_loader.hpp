#pragma once
#include "component_interface.hpp"
#include "launcher/launcher.hpp"
#include "game/game.hpp"

class component_loader final
{
public:
	class premature_shutdown_trigger final : public std::exception
	{
		[[nodiscard]] const char* what() const noexcept override
		{
			return "Premature shutdown requested";
		}
	};

	template <typename T, launcher::game G>
	class installer final
	{
		static_assert(std::is_base_of<component_interface, T>::value, "component has invalid base class");

	public:
		installer()
		{
			register_component_factory([]() { return std::make_unique<T>(); }, G);
		}
	};

	template <typename T>
	static T* get()
	{
		for (const auto& component_ : get_components())
		{
			if (typeid(*component_.get()) == typeid(T))
			{
				return reinterpret_cast<T*>(component_.get());
			}
		}

		return nullptr;
	}

	static void register_component(std::unique_ptr<component_interface>&& component);
	static void register_component_factory(std::function<std::unique_ptr<component_interface>()> factory, launcher::game target_game);
	static void register_symbol(resolve_after_load_symbol_interface*, launcher::game target_game);
	static void create_components(launcher::game target_game);

	static bool post_start();
	static bool post_load();
	static bool post_avs_init();
	static void pre_destroy();
	static void clean();

	static void* load_import(const std::string& library, const std::string& function);

	static void trigger_premature_shutdown();

private:
	static std::vector<std::unique_ptr<component_interface>>& get_components();
	static std::vector<std::function<std::unique_ptr<component_interface>()>>& get_components_factories(launcher::game);
	static std::vector<resolve_after_load_symbol_interface *>& get_symbols(launcher::game);
};

#define REGISTER_COMPONENT(name, game)                          \
namespace														\
{																\
	static component_loader::installer<name, game> __component; \
}
