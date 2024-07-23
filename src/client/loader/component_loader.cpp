#include <std_include.hpp>
#include "component_loader.hpp"

void component_loader::register_component(std::unique_ptr<component_interface>&& component_)
{
	get_components().push_back(std::move(component_));
}

void component_loader::register_component_factory(std::function<std::unique_ptr<component_interface>()> factory, launcher::game target_game)
{
	if (target_game == launcher::game::all)
	{
		register_component(factory());
		return;
	}

	get_components_factories(target_game).push_back(factory);
}

void component_loader::register_symbol(resolve_after_load_symbol_interface* symbol, launcher::game target_game)
{
	get_symbols(target_game).push_back(symbol);
}

void component_loader::create_components(launcher::game target_game)
{
	for (auto& factory : get_components_factories(target_game))
		register_component(factory());
}

bool component_loader::post_start()
{
	static auto handled = false;
	if (handled) return true;
	handled = true;

	try
	{
		for (const auto& component_ : get_components())
		{
			component_->post_start();
		}
	}
	catch (premature_shutdown_trigger&)
	{
		return false;
	}

	return true;
}

bool component_loader::post_load()
{
	static auto handled = false;
	if (handled) return true;
	handled = true;

	clean();

	try
	{
		for (const auto& symbol_ : get_symbols(launcher::game::all))
		{
			symbol_->resolve();
		}

		for (const auto& symbol_ : get_symbols(game::environment::get_game()))
		{
			symbol_->resolve();
		}

		for (const auto& component_ : get_components())
		{
			component_->post_load();
		}
	}
	catch (premature_shutdown_trigger&)
	{
		return false;
	}

	return true;
}

bool component_loader::post_avs_init()
{
	static auto handled = false;
	if (handled) return true;
	handled = true;

	clean();

	try
	{
		for (const auto& component_ : get_components())
		{
			component_->post_avs_init();
		}
	}
	catch (premature_shutdown_trigger&)
	{
		return false;
	}

	return true;
}

void component_loader::pre_destroy()
{
	static auto handled = false;
	if (handled) return;
	handled = true;

	for (const auto& component_ : get_components())
	{
		component_->pre_destroy();
	}
}

void component_loader::clean()
{
	auto& components = get_components();
	for (auto i = components.begin(); i != components.end();)
	{
		if (!(*i)->is_supported())
		{
			(*i)->pre_destroy();
			i = components.erase(i);
		}
		else
		{
			++i;
		}
	}
}

void* component_loader::load_import(const std::string& library, const std::string& function)
{
	void* function_ptr = nullptr;

	for (const auto& component_ : get_components())
	{
		auto* const component_function_ptr = component_->load_import(library, function);
		if (component_function_ptr)
		{
			function_ptr = component_function_ptr;
		}
	}

	return function_ptr;
}

void component_loader::trigger_premature_shutdown()
{
	throw premature_shutdown_trigger();
}

std::vector<std::unique_ptr<component_interface>>& component_loader::get_components()
{
	using component_vector = std::vector<std::unique_ptr<component_interface>>;
	using component_vector_container = std::unique_ptr<component_vector, std::function<void(component_vector*)>>;

	static component_vector_container components(new component_vector, [](component_vector* component_vector)
		{
			pre_destroy();
			delete component_vector;
		});

	return *components;
}

std::vector<std::function<std::unique_ptr<component_interface>()>>& component_loader::get_components_factories(launcher::game game)
{
	using component_factory = std::function<std::unique_ptr<component_interface>()>;
	using component_factories = std::vector<component_factory>;
	using component_factories_container = std::array<component_factories, static_cast<size_t>(launcher::game::count)>;
	using component_factories_container_ptr = std::unique_ptr < component_factories_container, std::function<void(component_factories_container*)>>;

	static component_factories_container_ptr container(new component_factories_container, [](component_factories_container* factories)
		{
			delete factories;
		});

	return container->at(static_cast<size_t>(game));
}

std::vector<resolve_after_load_symbol_interface*>& component_loader::get_symbols(launcher::game game)
{
	using symbols = std::vector<resolve_after_load_symbol_interface*>;
	using symbols_container = std::array<symbols, static_cast<size_t>(launcher::game::count) + 1>;

	static symbols_container container{};

	return container.at(static_cast<size_t>(game));
}
