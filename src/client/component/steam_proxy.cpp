#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "steam_proxy.hpp"

#include <utils/nt.hpp>
#include <utils/flags.hpp>
#include <utils/string.hpp>
#include <utils/binary_resource.hpp>

#include "game/game.hpp"

#include "steam/interface.hpp"
#include "steam/steam.hpp"

namespace steam_proxy
{
	namespace
	{
		bool is_disabled()
		{
			static const auto disabled = utils::flags::has_flag("nosteam");
			return disabled;
		}
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			if (is_disabled())
			{
				return;
			}

			try
			{
				this->load_client();
			}
			catch (std::exception& e)
			{
				printf("Steam: %s\n", e.what());
			}
		}

		void pre_destroy() override
		{
			if (this->steam_client_module_)
			{
				if (this->steam_pipe_)
				{
					if (this->global_user_)
					{
						this->steam_client_module_.invoke<void>("Steam_ReleaseUser", this->steam_pipe_,
							this->global_user_);
					}

					this->steam_client_module_.invoke<bool>("Steam_BReleaseSteamPipe", this->steam_pipe_);
				}
			}
		}

		const utils::nt::library& get_overlay_module() const
		{
			return steam_overlay_module_;
		}

		bool set_status(const std::string& status)
		{
			if (!this->client_friends_)
				return false;

			auto result = this->client_friends_.invoke<bool, size_t, const char*, const char*>("SetRichPresence", app_id_, "steam_display", "#state");

			if (!result)
				return false;

			return this->client_friends_.invoke<bool, size_t, const char*, const char*>("SetRichPresence", app_id_, "state", status.data());
		}

		uint64_t get_steam_id()
		{
			return this->steam_id_;
		}

		void start_mod(const std::string& title, const size_t app_id)
		{
			__try
			{
				this->start_mod_unsafe(title, app_id);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				this->do_cleanup();
			}
		}

	private:
		utils::nt::library steam_client_module_{};
		utils::nt::library steam_overlay_module_{};

		steam::interface client_engine_ {};
		steam::interface client_user_ {};
		steam::interface client_utils_ {};
		steam::interface client_friends_ {};

		size_t app_id_ = 0;
		uint64_t steam_id_ = 0xFFFFFFFFDEADBEEFul;

		void* steam_pipe_ = nullptr;
		void* global_user_ = nullptr;

		HANDLE event = nullptr;

		void* load_client_engine() const
		{
			if (!this->steam_client_module_) return nullptr;

			for (auto i = 1; ; ++i)
			{
				std::string name = utils::string::va("CLIENTENGINE_INTERFACE_VERSION%03i", i);
				auto* const client_engine = this->steam_client_module_
					.invoke<void*>("CreateInterface", name.data(), nullptr);
				if (client_engine) return client_engine;
			}
		}

		void load_client()
		{
			const std::filesystem::path steam_path = steam::get_steam_install_path();
			if (steam_path.empty()) return;

			utils::nt::library::load(steam_path / "tier0_s64.dll");
			utils::nt::library::load(steam_path / "vstdlib_s64.dll");

			this->steam_client_module_ = utils::nt::library::load(steam_path / "steamclient64.dll");
			if (!this->steam_client_module_) return;

			this->client_engine_ = load_client_engine();
			if (!this->client_engine_) return;

			this->steam_pipe_ = this->steam_client_module_.invoke<void*>("Steam_CreateSteamPipe");
			this->global_user_ = this->steam_client_module_.invoke<void*>(
				"Steam_ConnectToGlobalUser", this->steam_pipe_);
			this->client_user_ = this->client_engine_.invoke<void*>(8, this->steam_pipe_, this->global_user_);
			// GetIClientUser
			this->client_utils_ = this->client_engine_.invoke<void*>(14, this->steam_pipe_); // GetIClientUtils
			this->client_friends_ = this->client_engine_.invoke<void*>(13, this->steam_pipe_); // GetIClientFriends

			this->client_user_.invoke<void, uint64_t*>("GetSteamID", &this->steam_id_);
		}

		void start_mod_unsafe(const std::string& title, size_t app_id)
		{
			if (!this->client_utils_ || !this->client_user_) return;

			auto event_name = utils::string::va("__laochan_runner__%d", GetCurrentProcessId());

			if (!event)
			{
				event = CreateEventA(NULL, true, false, event_name);

				if (!event) [[unlikely]]
					return;
			}

			// close last runner
			PulseEvent(event);

			if (!this->client_user_.invoke<bool>("BIsSubscribedApp", app_id))
			{
				app_id = 480; // Spacewar
			}

			this->client_utils_.invoke<void>("SetAppIDForCurrentPipe", app_id, false);

			char our_directory[MAX_PATH] = { 0 };
			GetCurrentDirectoryA(sizeof(our_directory), our_directory);

			utils::binary_resource runner_file(nullptr, RUNNER, "laochan-runner.exe");

			const auto path = runner_file.get_extracted_file();
			const std::string cmdline = utils::string::va("\"%s\" -event %s -process %d", path.data(), event_name, GetCurrentProcessId());

			steam::game_id game_id;
			game_id.raw.type = 1; // k_EGameIDTypeGameMod
			game_id.raw.app_id = app_id & 0xFFFFFF;

			this->app_id_ = game_id.raw.app_id;

			const auto* mod_id = "LCAN";
			game_id.raw.mod_id = *reinterpret_cast<const unsigned int*>(mod_id) | 0x80000000;

			this->client_user_.invoke<bool>("SpawnProcess", path.data(), cmdline.data(), our_directory,
				&game_id.bits, title.data(), 0, 0, 0);
		}

		void do_cleanup()
		{
			this->client_engine_ = nullptr;
			this->client_user_ = nullptr;
			this->client_utils_ = nullptr;

			this->steam_pipe_ = nullptr;
			this->global_user_ = nullptr;

			this->steam_client_module_ = utils::nt::library{ nullptr };
		}
	};

	const utils::nt::library& get_overlay_module()
	{
		// TODO: Find a better way to do this
		return component_loader::get<component>()->get_overlay_module();
	}

	void set_status(const std::string& status)
	{
		auto* const comp = component_loader::get<component>();

		if (!comp)
			return;

		return comp->start_mod(status, 980610);
	}

	uint64_t get_steam_id()
	{
		auto* const comp = component_loader::get<component>();

		if (!comp)
			return false;

		return comp->get_steam_id();
	}
}

REGISTER_COMPONENT(steam_proxy::component, launcher::game::all)
