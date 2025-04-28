#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <game/game.hpp>
#include <launcher/launcher.hpp>
#include <DbgHelp.h>

namespace sdvx::patches
{
	bool is_signature_valid()
	{
		return true;
	}

	BOOL is_debugger_present()
	{
		return false;
	}


	struct extdrmfs_data
	{
		char type[8];
		void* a;
		void* b;
		void* c;
		void* d;
	};

	int on_mount(const char* mountpoint, const char* fsroot, const char* fstype, void* data)
	{
		if (mountpoint)
		{
			printf("mount %s to %s with type %s\n", fsroot, mountpoint, fstype);

			if (false && fstype == "extdrmfs"s)
			{
				auto* d = reinterpret_cast<extdrmfs_data*>(data);
				printf("mounting extdrmfs %s\n", d->type);

				char buffer1[256];
				char buffer2[256];
				RtlZeroMemory(buffer1, 256);
				RtlZeroMemory(buffer2, 256);

				char buffer3[256];
				char buffer4[256];

				auto size_1 = utils::hook::invoke<size_t>(d->b, buffer3, buffer1);
				auto size_2 = utils::hook::invoke<size_t>(d->c, buffer4, buffer2);

				std::string str1{ buffer1, buffer1 + size_1 };
				std::string str2{ buffer2, buffer2 + size_2 };

				printf("buffer1: %s\nbuffer2: %s\n", str1.data(), str2.data());
			}
		}

		return avs2::fs_mount(mountpoint, fsroot, fstype, data);
	}

	utils::hook::detour hash_update_d;
	utils::hook::detour hash_finish_d;

	void dump_hex(const void* data, size_t size) 
	{
		char ascii[17];
		size_t i, j;
		ascii[16] = '\0';
		for (i = 0; i < size; ++i)
		{
			printf("%02X ", ((unsigned char*)data)[i]);
			if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~')
			{
				ascii[i % 16] = ((unsigned char*)data)[i];
			}
			else
			{
				ascii[i % 16] = '.';
			}
			if ((i + 1) % 8 == 0 || i + 1 == size)
			{
				printf(" ");
				if ((i + 1) % 16 == 0)
				{
					printf("|  %s \n", ascii);
				}
				else if (i + 1 == size)
				{
					ascii[(i + 1) % 16] = '\0';
					if ((i + 1) % 16 <= 8)
					{
						printf(" ");
					}
					for (j = (i + 1) % 16; j < 16; ++j)
					{
						printf("   ");
					}
					printf("|  %s \n", ascii);
				}
			}
		}
	}

	void* path_hash = nullptr;

	void hash_update(void* digest, const char* data, int size)
	{
		if (data == "eyrfqe7"s)
		{
			path_hash = digest;
		}

		if (digest == path_hash)
		{
			printf("hash update\n");
			dump_hex(data, size);
			printf("\n");
		}

		hash_update_d.invoke<void>(digest, data, size);
	}

	void hash_finish(void* digest, uint8_t* hash, int size)
	{
		if (digest == path_hash)
		{
			printf("hash finish\n");
			path_hash = nullptr;
		}

		hash_finish_d.invoke<void>(digest, hash, size);
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			utils::nt::library avs2{ "avs2-core.dll" };
			hash_update_d.create(avs2.get_proc<size_t>("XCgsqzn0000141"), hash_update);
			hash_finish_d.create(avs2.get_proc<size_t>("XCgsqzn0000142"), hash_finish);

			utils::nt::library libeacnet{ "libeacnet.dll" };
			
			// disable signature check
			utils::hook::jump(libeacnet.get_ptr() + 0xF9F0, is_signature_valid, true);
		}

		void* load_import(const std::string& library, const std::string& function) override
		{
			// disable debugger trap
			if (function == "IsDebuggerPresent")
			{
				return is_debugger_present;
			}
			else if (function == "#76")
			{
				return on_mount;
			}

			return nullptr;
		}
	};
}

REGISTER_COMPONENT(sdvx::patches::component, launcher::game::sdvx)
