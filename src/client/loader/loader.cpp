#include <std_include.hpp>
#include "loader.hpp"

#include <utils/string.hpp>
#include <utils/hook.hpp>

utils::nt::library loader::load_library(const std::string& filename) const
{
	const auto target = utils::nt::library::load(filename);
	if (!target)
	{
		throw std::runtime_error{ "Failed to load game binary!" };
	}

	this->load_imports(target);
	this->load_tls(target);

	return target;
}

void loader::set_import_resolver(const std::function<void* (const std::string&, const std::string&)>& resolver)
{
	this->import_resolver_ = resolver;
}

void loader::load_imports(const utils::nt::library& target) const
{
	auto* const import_directory = &target.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

	auto* descriptor = PIMAGE_IMPORT_DESCRIPTOR(target.get_ptr() + import_directory->VirtualAddress);

	while (descriptor->Name)
	{
		std::string name = LPSTR(target.get_ptr() + descriptor->Name);

		auto* name_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->OriginalFirstThunk);
		auto* address_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->FirstThunk);

		if (!descriptor->OriginalFirstThunk)
		{
			name_table_entry = reinterpret_cast<uintptr_t*>(target.get_ptr() + descriptor->FirstThunk);
		}

		while (*name_table_entry)
		{
			FARPROC function = nullptr;
			std::string function_name;
			const char* function_procname;

			if (IMAGE_SNAP_BY_ORDINAL(*name_table_entry))
			{
				function_name = "#" + std::to_string(IMAGE_ORDINAL(*name_table_entry));
				function_procname = MAKEINTRESOURCEA(IMAGE_ORDINAL(*name_table_entry));
			}
			else
			{
				auto * import = PIMAGE_IMPORT_BY_NAME(target.get_ptr() + *name_table_entry);
				function_name = import->Name;
				function_procname = function_name.data();
			}

			if (this->import_resolver_) function = FARPROC(this->import_resolver_(name, function_name));
			if (!function)
			{
				auto library = utils::nt::library::load(name);
				if (library)
				{
					function = GetProcAddress(library, function_procname);
				}
			}

			if (!function)
			{
				throw std::runtime_error(utils::string::va("Unable to load import '%s' from library '%s'",
					function_name.data(), name.data()));
			}

			utils::hook::set(address_table_entry, reinterpret_cast<uintptr_t>(function));

			name_table_entry++;
			address_table_entry++;
		}

		descriptor++;
	}
}

// well, this actually works
void loader::load_tls(const utils::nt::library& target) const
{
	if (!target.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
		return;

	auto* const source_tls = reinterpret_cast<PIMAGE_TLS_DIRECTORY>(target.get_ptr() + target.get_optional_header()
		->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);

	const auto tls_size = source_tls->EndAddressOfRawData - source_tls->StartAddressOfRawData;
	auto* tls_buffer = std::malloc(4096);

	std::memmove(tls_buffer, PVOID(source_tls->StartAddressOfRawData), tls_size);

	auto* const tls_ptr = reinterpret_cast<void**>(
#if _WIN64
		__readgsqword(0x58)
#else
		__readfsdword(0x2C)
#endif
	);

	*tls_ptr = tls_buffer;
}
