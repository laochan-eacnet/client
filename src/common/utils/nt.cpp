#include "nt.hpp"
#include "memory.hpp"
#include "signature.hpp"

#include <gsl/gsl>

namespace utils::nt
{
	library library::load(const std::string& name)
	{
		return library{ name };
	}

	library library::load(const std::filesystem::path& path)
	{
		return library{ path.generic_string() };
	}

	library library::get_by_address(void* address)
	{
		HMODULE handle = nullptr;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
						   static_cast<LPCSTR>(address), &handle);
		return library(handle);
	}

	library::library()
	{
		this->module_ = GetModuleHandleA(nullptr);
	}

	library::library(const std::string& name)
	{
		this->module_ = GetModuleHandleA(name.data());
		if (!this->module_) this->module_ = LoadLibraryA(name.data());
	}

	library::library(const HMODULE handle)
	{
		this->module_ = handle;
	}

	bool library::operator==(const library& obj) const
	{
		return this->module_ == obj.module_;
	}

	library::operator bool() const
	{
		return this->is_valid();
	}

	library::operator HMODULE() const
	{
		return this->get_handle();
	}

	PIMAGE_NT_HEADERS library::get_nt_headers() const
	{
		if (!this->is_valid()) return nullptr;
		return reinterpret_cast<PIMAGE_NT_HEADERS>(this->get_ptr() + this->get_dos_header()->e_lfanew);
	}

	PIMAGE_DOS_HEADER library::get_dos_header() const
	{
		return reinterpret_cast<PIMAGE_DOS_HEADER>(this->get_ptr());
	}

	PIMAGE_OPTIONAL_HEADER library::get_optional_header() const
	{
		if (!this->is_valid()) return nullptr;
		return &this->get_nt_headers()->OptionalHeader;
	}

	std::vector<PIMAGE_SECTION_HEADER> library::get_section_headers() const
	{
		std::vector<PIMAGE_SECTION_HEADER> headers;

		auto nt_headers = this->get_nt_headers();
		auto section = IMAGE_FIRST_SECTION(nt_headers);

		for (uint16_t i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i, ++section)
		{
			if (section) headers.push_back(section);
			else OutputDebugStringA("There was an invalid section :O");
		}

		return headers;
	}

	std::uint8_t* library::get_ptr() const
	{
		return reinterpret_cast<std::uint8_t*>(this->module_);
	}

	void library::unprotect() const
	{
		if (!this->is_valid()) return;

		DWORD protection;
		VirtualProtect(this->get_ptr(), this->get_optional_header()->SizeOfImage, PAGE_EXECUTE_READWRITE,
					   &protection);
	}

	size_t library::get_relative_entry_point() const
	{
		if (!this->is_valid()) return 0;
		return this->get_nt_headers()->OptionalHeader.AddressOfEntryPoint;
	}

	FARPROC library::get_entry_point() const
	{
		if (!this->is_valid()) return nullptr;
		return reinterpret_cast<FARPROC>(this->get_ptr() + this->get_relative_entry_point());
	}

	bool library::is_valid() const
	{
		return this->module_ != nullptr && this->get_dos_header()->e_magic == IMAGE_DOS_SIGNATURE;
	}

	std::string library::get_name() const
	{
		if (!this->is_valid()) return "";

		auto path = this->get_path().generic_string();
		const auto pos = path.find_last_of("/\\");
		if (pos == std::string::npos) return path;

		return path.substr(pos + 1);
	}

	std::filesystem::path library::get_path() const
	{
		if (!this->is_valid()) return "";

		wchar_t name[MAX_PATH] = { 0 };
		GetModuleFileNameW(this->module_, name, sizeof name);

		return name;
	}

	std::string library::get_folder() const
	{
		if (!this->is_valid()) return "";

		const auto path = std::filesystem::path(this->get_path());
		return path.parent_path().generic_string();
	}

	std::string library::get_version(const std::filesystem::path& path)
	{
		DWORD handle = 0;

		auto p = path.generic_wstring();
		auto size = GetFileVersionInfoSizeW(p.data(), &handle);

		if (!size || handle) return {};

		auto buffer = memory::allocate(size);
		auto _ = gsl::finally([=] {
			memory::free(buffer);
		});

		if (!GetFileVersionInfoW(p.data(), handle, size, buffer))
		{
			return {};
		}

		LPVOID out_buffer;
		UINT out_size;

		if (!VerQueryValueA(buffer, "\\StringFileInfo\\041104b0\\ProductVersion", &out_buffer, &out_size))
		{
			return {};
		}

		return std::string{ reinterpret_cast<const char*>(out_buffer) };
	}

	std::string library::get_version() const
	{
		return library::get_version(get_path());
	}

	void library::free()
	{
		if (this->is_valid())
		{
			FreeLibrary(this->module_);
			this->module_ = nullptr;
		}
	}

	HMODULE library::get_handle() const
	{
		return this->module_;
	}

	void** library::get_iat_entry(const std::string& module_name, const std::string& proc_name) const
	{
		return this->get_iat_entry(module_name, proc_name.data());
	}

	void** library::get_iat_entry(const std::string& module_name, const char* proc_name) const
	{
		if (!this->is_valid()) return nullptr;

		const library other_module(module_name);
		if (!other_module.is_valid()) return nullptr;

		auto* const target_function = other_module.get_proc<void*>(proc_name);
		if (!target_function) return nullptr;

		const auto* header = this->get_optional_header();
		if (!header) return nullptr;

		auto* import_descriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(this->get_ptr() + header->DataDirectory
																			 [IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		while (import_descriptor->Name)
		{
			if (!_stricmp(reinterpret_cast<char*>(this->get_ptr() + import_descriptor->Name), module_name.data()))
			{
				auto* original_thunk_data = reinterpret_cast<PIMAGE_THUNK_DATA>(import_descriptor->
																				OriginalFirstThunk + this->get_ptr());
				auto* thunk_data = reinterpret_cast<PIMAGE_THUNK_DATA>(import_descriptor->FirstThunk + this->
																	   get_ptr());

				while (original_thunk_data->u1.AddressOfData)
				{
					if (thunk_data->u1.Function == reinterpret_cast<uint64_t>(target_function))
					{
						return reinterpret_cast<void**>(&thunk_data->u1.Function);
					}

					const size_t ordinal_number = original_thunk_data->u1.AddressOfData & 0xFFFFFFF;

					if (ordinal_number <= 0xFFFF)
					{
						auto* proc = GetProcAddress(other_module.module_, reinterpret_cast<char*>(ordinal_number));
						if (reinterpret_cast<void*>(proc) == target_function)
						{
							return reinterpret_cast<void**>(&thunk_data->u1.Function);
						}
					}

					++original_thunk_data;
					++thunk_data;
				}

				//break;
			}

			++import_descriptor;
		}

		return nullptr;
	}

	std::vector<uint8_t*> library::match_sigs(const std::string& pattern) const
	{
		hook::signature sig{ pattern, *this };
		auto result = sig.process(false);

		return result;
	}

	uint8_t* library::match_sig(const std::string& pattern) const
	{
		hook::signature sig{ pattern, *this };
		auto result = sig.process(true);

		return result.size() ? result[0] : nullptr;
	}

	void raise_hard_exception()
	{
		int data = false;
		const library ntdll("ntdll.dll");
		ntdll.invoke_pascal<void>("RtlAdjustPrivilege", 19, true, false, &data);
		ntdll.invoke_pascal<void>("NtRaiseHardError", 0xC000007B, 0, nullptr, nullptr, 6, &data);
	}

	std::string load_resource(HMODULE module, const int id)
	{
		auto* const res = FindResource(module, MAKEINTRESOURCE(id), RT_RCDATA);
		if (!res) return {};

		auto* const handle = LoadResource(module, res);
		if (!handle) return {};

		return std::string(LPSTR(LockResource(handle)), SizeofResource(module, res));
	}

	void relaunch_self()
	{
		const utils::nt::library self;

		STARTUPINFOA startup_info;
		PROCESS_INFORMATION process_info;

		ZeroMemory(&startup_info, sizeof(startup_info));
		ZeroMemory(&process_info, sizeof(process_info));
		startup_info.cb = sizeof(startup_info);

		char current_dir[MAX_PATH];
		GetCurrentDirectoryA(sizeof(current_dir), current_dir);
		auto* const command_line = GetCommandLineA();

		CreateProcessA(self.get_path().generic_string().data(), command_line, nullptr, nullptr, false, NULL, nullptr, current_dir,
					   &startup_info, &process_info);

		if (process_info.hThread && process_info.hThread != INVALID_HANDLE_VALUE) CloseHandle(process_info.hThread);
		if (process_info.hProcess && process_info.hProcess != INVALID_HANDLE_VALUE) CloseHandle(process_info.hProcess);
	}

	void terminate(const uint32_t code)
	{
		TerminateProcess(GetCurrentProcess(), code);
	}
}
