#pragma once
#include <utils/nt.hpp>

class loader final
{
public:
	utils::nt::library load_library(const std::string& filename) const;

	void set_import_resolver(const std::function<void* (const std::string&, const std::string&)>& resolver);

private:
	std::function<void* (const std::string&, const std::string&)> import_resolver_;

	void load_imports(const utils::nt::library& target) const;
	void load_tls(const utils::nt::library& target) const;
};
