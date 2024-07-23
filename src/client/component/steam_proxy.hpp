#pragma once
#include <utils/nt.hpp>

namespace steam_proxy
{
	const utils::nt::library& get_overlay_module();
	void set_status(const std::string& status);
	uint64_t get_steam_id();
	void load_steam();
}
