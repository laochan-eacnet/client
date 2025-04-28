#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>

#include <game/game.hpp>
#include <launcher/launcher.hpp>

namespace iidx::env
{
	bool use_exp()
	{
		const static bool result = ([]() {
			return game::environment::get_param("IIDX_EXP_FEATURE") == "1";
		})();

		return result;
	}

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

		utils::hook::set<uint32_t>(iidx::language.get() + 4, language);
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
			
			CONTEXT ctx{ 0 };
			ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
			auto thread = GetCurrentThread();
			if (GetThreadContext(thread, &ctx))
			{
				ctx.Dr0 = reinterpret_cast<DWORD64>(iidx::language.get());
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
