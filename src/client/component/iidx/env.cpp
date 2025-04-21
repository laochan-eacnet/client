#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include <game/game.hpp>
#include <launcher/launcher.hpp>

namespace iidx::env
{
	const char* get_cmdline()
	{
		static auto cmdline = ([]
			{
				auto args = "bm2dx.exe -t "s;
				args += game::environment::get_param("LAOCHAN_TOKEN");

				if (game::environment::get_param("IIDX_SOUND_MODE") == "1")
					args += " --asio ";

				printf("Starting game with cmdline: %s\n", args.data());
				return args;
			}
		)();

		return cmdline.data();
	}

	utils::hook::detour load_ifs;
	char __fastcall load_ifs_hook(char* a1, __int64 a2, int a3, const char* a4, unsigned int a5, char a6)
	{
		printf("load ifs %s\n", a4);
		return load_ifs.invoke<char>(a1, a2, a3, a4, a5, a6);
	}

	LONG WINAPI set_language_hook(struct _EXCEPTION_POINTERS* ExceptionInfo)
	{
		if (ExceptionInfo->ExceptionRecord->ExceptionCode != EXCEPTION_SINGLE_STEP)
			return EXCEPTION_CONTINUE_SEARCH;

		// set language
		static auto language = ([]
			{
				return std::stoi(game::environment::get_param("IIDX_LANGUAGE"));
			}
		)();

		utils::hook::set<uint32_t>(0x1405F0D24, language);
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		}

		void post_load() override
		{
			auto version = game::environment::get_version();
			if (version != IIDX_TARGET_VERSION)
			{
				throw std::runtime_error(utils::string::va("Unsupported version %s\nSupported version is " IIDX_TARGET_VERSION ".", version.data()));
			}

			load_ifs.create(0x140224290, load_ifs_hook);
			// init_thread_footer.create(0x14035EFA4, init_thread_footer_hook);
			
			CONTEXT ctx{ 0 };
			ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
			auto thread = GetCurrentThread();
			if (GetThreadContext(thread, &ctx))
			{
				ctx.Dr0 = 0x1405F0D20;
				ctx.Dr7 = 1 | 1 << 16;

				SetThreadContext(thread, &ctx);
			}

			AddVectoredExceptionHandler(1, set_language_hook);
		}

		void* load_import(const std::string& library, const std::string& function) override
		{
			if (function == "GetCommandLineA")
			{
				return get_cmdline;
			}

			return nullptr;
		}
	};
}

REGISTER_COMPONENT(iidx::env::component, launcher::game::iidx)
