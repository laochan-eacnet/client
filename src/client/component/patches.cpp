#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <game/game.hpp>
#include <launcher/launcher.hpp>
#include <DbgHelp.h>

char asio_name[0x2048];

namespace patches
{
	bool is_signature_valid()
	{
		return true;
	}

	bool is_debugger_present()
	{
		return false;
	}

	const char* get_commandline()
	{
		return launcher::get_args();
	}

	const char* get_service_url()
	{
		return launcher::get_service_address.data();
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			utils::nt::library game{};
			utils::nt::library libeacnet{ "libeacnet.dll" };
			utils::nt::library kernel32{ "kernel32.dll" };

			// disable signature check
			utils::hook::jump(libeacnet.get_ptr() + 0xF9E0, is_signature_valid, true);

			// change service url
			utils::hook::jump(libeacnet.get_ptr() + 0x11EC0, get_service_url, true);

			// disable debugger trap
			utils::hook::jump(kernel32.get_proc<void*>("IsDebuggerPresent"), is_debugger_present, true);
			
			// disable anticheat thread
			utils::hook::nop(0x140003AC2, 5);

			// enable console
			utils::hook::set<uint16_t>(0x14000278c, 0x44ff);

			utils::hook::iat(game, "kernel32.dll", "GetCommandLineA", get_commandline);
		}

#ifdef DUMP_DATA
		void post_load() override
		{
			game::avs_stat stat = { 0 };

			auto file = game::avs_fs_open("/data/__metadata.metatxt", 1, 420);

			game::avs_fs_fstat(file, &stat);
			std::string buffer;
			buffer.resize(stat.filesize + 1);

			game::avs_fs_read(file, buffer.data(), stat.filesize);
			game::avs_fs_close(file);

			auto metadata = nlohmann::json::parse(buffer);
			for (auto& entry : metadata["files"])
			{
				auto src = "/data" + entry["spath"].get<std::string>();

				std::filesystem::path dest = "./data_dump" + entry["spath"].get<std::string>();
				auto dest_str = dest.make_preferred().string();
				std::replace(dest_str.begin(), dest_str.end(), '\\', '/');

				std::filesystem::create_directories(dest.parent_path());
				printf("%s\n", dest_str.data());

				auto tfile = game::avs_fs_open(src.data(), 1, 420);

				if (tfile < 0) {
					printf("error dumping %s: %x\n", src.data(), tfile);
					continue;
				}

				game::avs_fs_fstat(tfile, &stat);
				std::string tbuffer;
				tbuffer.resize(stat.filesize + 1);

				game::avs_fs_read(tfile, tbuffer.data(), stat.filesize);
				game::avs_fs_close(tfile);

				FILE* of;
				fopen_s(&of, dest_str.data(), "wb");
				fwrite(tbuffer.data(), stat.filesize, 1, of);
				fclose(of);
			}
		}
#endif
	};
}

REGISTER_COMPONENT(patches::component)
