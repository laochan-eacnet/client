#include <std_include.hpp>

#include "loader.hpp"
#include "game/game.hpp"

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
	auto* descriptor = target.get_data_directory<IMAGE_IMPORT_DESCRIPTOR>(IMAGE_DIRECTORY_ENTRY_IMPORT); 

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

//
// saved my day, thank you
// https://github.com/polycone/pe-loader/blob/master/loader/src/loader/tls_support.cpp#L68
//
IMAGE_TLS_DIRECTORY* find_ldrp_tls()
{
	utils::nt::library self{};
	auto main_tls = self.get_data_directory<IMAGE_TLS_DIRECTORY>(IMAGE_DIRECTORY_ENTRY_TLS);

	auto heap = reinterpret_cast<char*>(GetProcessHeap());
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery(heap, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

	auto heap_size = mbi.RegionSize;
	
	for (auto p = heap; p < heap + heap_size; p += sizeof(size_t))
	{
		if (memcmp(p, main_tls, sizeof(IMAGE_TLS_DIRECTORY)))
			continue;

		return reinterpret_cast<IMAGE_TLS_DIRECTORY *>(p);
	}

	return nullptr;
}

void loader::load_tls(const utils::nt::library& target) const
{
	if (!target.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
		return;

	auto ldr_tls = find_ldrp_tls();
	auto target_tls = target.get_data_directory<IMAGE_TLS_DIRECTORY>(IMAGE_DIRECTORY_ENTRY_TLS);

	const auto tls_size = target_tls->EndAddressOfRawData - target_tls->StartAddressOfRawData;
	const auto tls_index = *reinterpret_cast<DWORD*>(ldr_tls->AddressOfIndex);

	auto* const tls_base = *reinterpret_cast<LPVOID*>(__readgsqword(0x58) + 8ull * tls_index);
	std::memmove(tls_base, PVOID(target_tls->StartAddressOfRawData), tls_size);

	utils::hook::set(target_tls->AddressOfIndex, tls_index);
	ldr_tls->AddressOfIndex = target_tls->AddressOfIndex;
	ldr_tls->StartAddressOfRawData = target_tls->StartAddressOfRawData;
	ldr_tls->EndAddressOfRawData = target_tls->EndAddressOfRawData;
	ldr_tls->AddressOfCallBacks = target_tls->AddressOfCallBacks;

	for (auto callback = reinterpret_cast<uintptr_t*>(ldr_tls->AddressOfCallBacks); *callback; callback++)
	{
		utils::hook::invoke<void>(callback, target.get_ptr(), DLL_PROCESS_ATTACH, nullptr);
	}
}
