#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include <launcher/launcher.hpp>

namespace gitadora::patches
{
	bool is_signature_valid()
	{
		return true;
	}

	BOOL is_debugger_present()
	{
		return false;
	}

	// force exit anti debug thread
	void check_remote_debugger_preset(HANDLE, PBOOL preset)
	{
		ExitThread(0);
	}

	void* get_proc_address(HMODULE module, const char* name)
	{
		if (name == "IsDebuggerPresent"s)
			return is_debugger_present;

		if (name == "CheckRemoteDebuggerPresent"s)
			return check_remote_debugger_preset;

		return GetProcAddress(module, name);
	}

	const char* __fastcall get_service_url()
	{
		static auto service_url = ([]
		{
			return game::environment::get_param("LAOCHAN_SERVER_URL");
		}
								   )();

		return service_url.data();
	}

#ifdef GITADORA_ALTER_CONFIG
	int modify_config(avs2::node_ptr node, avs2::node_type type, void* data, uint32_t data_size)
	{
		char name_buf[64];
		avs2::property_node_name(node, name_buf, sizeof(name_buf));

		printf("read %s\n", name_buf);

		if (name_buf == "fs_rate"s)
		{
			utils::hook::set<int32_t>(data, 120);
			return 0;
		}

		if (name_buf == "flip_interval"s)
		{
			std::memcpy(data, "IMMEDIATE", 10);
			return 0;
		}

		return avs2::property_node_read(node, type, data, data_size);
	}
#endif

	enum LANGUAGE : uint32_t
	{
		JAPANESE = 0,
		ENGLISH = 1,
		KOREAN = 2,
		TRADITIONAL_CHINESE = 3,
		SIMPLIFIED_CHINESE = 4,
	};

	LANGUAGE get_language()
	{
		static auto language = ([]
		{
			auto language = game::environment::get_param("GITADORA_LANGUAGE");

			if (language.size())
			{
				return std::stoi(language);
			}

			return 1;
		})();

		return static_cast<LANGUAGE>(language);
	}

	uint8_t* get_language_return_pos = nullptr;
	auto* const get_language_stub()
	{
		return utils::hook::assemble([](utils::hook::assembler& a)
		{
			a.sub(rsp, 0x20);
			a.mov(edx, get_language());
			a.mov(rsi, rcx);
			a.jmp(get_language_return_pos);
		});
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			utils::nt::library libeacnet{ "libeacnet.dll" };

			// disable signature check
			utils::hook::jump(libeacnet.get_ptr() + 0xF9E0, is_signature_valid, true);

			// change service url
			utils::hook::jump(libeacnet.get_ptr() + 0x11EC0, get_service_url, true);

			auto sig_fps_limit = game::environment::get_module().match_sig("48 83 F8 0D");
			if (!sig_fps_limit)
				throw std::runtime_error{ "fps limit pattern not found" };

			utils::hook::set<uint8_t>(sig_fps_limit + 4, 0xEB);

			auto sig_language = game::environment::get_module().match_sig("83 EA 01 74 38");
			if (!sig_language)
				throw std::runtime_error{ "language pattern not found" };

			get_language_return_pos = sig_language;
			utils::hook::jump(sig_language - 13, get_language_stub(), true);
		}

		void* load_import(const std::string& library, const std::string& function) override
		{
			// disable debugger trap
			if (function == "IsDebuggerPresent")
			{
				return is_debugger_present;
			}
			else if (function == "GetProcAddress")
			{
				return get_proc_address;
			}
#ifdef GITADORA_ALTER_CONFIG
			else if (function == "#172")
			{
				return modify_config;
			}
#endif

			return nullptr;
		}
	};
}

REGISTER_COMPONENT(gitadora::patches::component, launcher::game::gitadora)
