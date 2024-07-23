#pragma once

class component_interface
{
public:
	virtual ~component_interface()
	{
	}

	virtual void post_start()
	{
	}

	virtual void post_load()
	{
	}

	virtual void post_avs_init()
	{
	}

	virtual void pre_destroy()
	{
	}

	virtual void* load_import([[maybe_unused]] const std::string& library, [[maybe_unused]] const std::string& function)
	{
		return nullptr;
	}

	virtual bool is_supported()
	{
		return true;
	}
};

class resolve_after_load_symbol_interface
{
public:
	virtual void resolve() = 0;
};
