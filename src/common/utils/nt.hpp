#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// min and max is required by gdi, therefore NOMINMAX won't work
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include <string>
#include <functional>
#include <filesystem>

namespace utils::nt
{
	class library final
	{
	public:
		static library load(const std::string& name);
		static library load(const std::filesystem::path& path);
		static library get_by_address(void* address);
		static std::string get_version(const std::filesystem::path& path);

		library();
		explicit library(const std::string& name);
		explicit library(HMODULE handle);

		library(const library& a) : module_(a.module_)
		{
		}

		bool operator!=(const library& obj) const { return !(*this == obj); };
		bool operator==(const library& obj) const;

		operator bool() const;
		operator HMODULE() const;

		void unprotect() const;
		FARPROC get_entry_point() const;
		size_t get_relative_entry_point() const;

		bool is_valid() const;
		std::filesystem::path get_path() const;
		std::string get_name() const;
		std::string get_folder() const;
		std::string get_version() const;
		std::uint8_t* get_ptr() const;
		void free();

		HMODULE get_handle() const;

		template <typename T>
		T get_proc(const std::string& process) const
		{
			if (!this->is_valid()) T{};
			return reinterpret_cast<T>(GetProcAddress(this->module_, process.data()));
		}

		template <typename T>
		T get_proc(const char* name) const
		{
			if (!this->is_valid()) T{};
			return reinterpret_cast<T>(GetProcAddress(this->module_, name));
		}

		template <typename T>
		std::function<T> get(const std::string& process) const
		{
			if (!this->is_valid()) return std::function<T>();
			return static_cast<T*>(this->get_proc<void*>(process));
		}

		template <typename T, typename... Args>
		T invoke(const std::string& process, Args ... args) const
		{
			auto method = this->get<T(__cdecl)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		template <typename T, typename... Args>
		T invoke_pascal(const std::string& process, Args ... args) const
		{
			auto method = this->get<T(__stdcall)(Args ...)>(process);
			if (method) return method(args...);
			return T();
		}

		template <typename T, typename... Args>
		T invoke_this(const std::string& process, void* this_ptr, Args ... args) const
		{
			auto method = this->get<T(__thiscall)(void*, Args ...)>(this_ptr, process);
			if (method) return method(args...);
			return T();
		}

		std::vector<PIMAGE_SECTION_HEADER> get_section_headers() const;

		PIMAGE_NT_HEADERS get_nt_headers() const;
		PIMAGE_DOS_HEADER get_dos_header() const;
		PIMAGE_OPTIONAL_HEADER get_optional_header() const;

		template <typename T>
		const T* get_data_directory(int directory) const
		{
			return reinterpret_cast<const T*>(this->get_ptr() + this->get_optional_header()->DataDirectory[directory].VirtualAddress);
		}

		void** get_iat_entry(const std::string& module_name, const std::string& proc_name) const;
		void** get_iat_entry(const std::string& module_name, const char* name) const;

		std::vector<uint8_t*> match_sigs(const std::string& pattern) const;
		uint8_t* match_sig(const std::string& pattern) const;
	private:
		HMODULE module_;
	};

	__declspec(noreturn) void raise_hard_exception();
	std::string load_resource(HMODULE module, int id);

	void relaunch_self();
	__declspec(noreturn) void terminate(uint32_t code = 0);
}
