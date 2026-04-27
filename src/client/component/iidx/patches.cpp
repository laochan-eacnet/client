#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <game/game.hpp>
#include <launcher/launcher.hpp>

namespace iidx::patches
{
	char asio_name[0x2048];

	const char* __fastcall get_service_url(void* _this, bool is_dev, bool is_kr)
	{
		static auto service_url = ([]
		{
			return game::environment::get_param("LAOCHAN_SERVER_URL");
		}
								   )();

		return service_url.data();
	}

    // todo: configurable
    int get_asio_channel_count()
    {
		return 8;
    }

	utils::hook::detour init_superstep_sound_hook;
	uint64_t __fastcall init_superstep_sound_stub(void* _this, int /* use_asio */, int sample_rate, int16_t bitrate_device, uint32_t bitrate, int channels, int exclusive)
	{
		// use 7.1ch output, for cabient
		if (init_superstep_sound_hook.invoke<uint64_t>(
			_this, true,
			sample_rate, bitrate_device, bitrate, get_asio_channel_count(), exclusive
		)) {
			return 1;
		}
		
		return init_superstep_sound_hook.invoke<uint64_t>(
			_this, true,
			sample_rate, bitrate_device, bitrate, channels, exclusive
		);
	}


	bool generate_music_list(avs2::property_ptr dst_prop, avs2::node_ptr /* dst_node */, avs2::node_ptr /*src_node*/, bool /*deep*/)
	{
		static const uint8_t zero_buffer[64]{};

		auto music_data = iidx::get_music_data();
		auto root = avs2::property_node_create(dst_prop, nullptr, avs2::NODE_TYPE_node, "/music_list", zero_buffer);

		avs2::property_node_create(dst_prop, root, avs2::NODE_TYPE_s32, "music_num", music_data->music_count);

		for (size_t i = 0; i < music_data->music_count; i++)
		{
			auto& music = music_data->musics[i];
			auto notebit = 0;

			for (size_t j = 0; j < 10; j++)
			{
				if (music.level[j])
					notebit |= 1 << j;
			}

			auto* node = avs2::property_node_create(dst_prop, root, avs2::NODE_TYPE_node, "music", zero_buffer);

			avs2::property_node_create(dst_prop, node, avs2::NODE_TYPE_s32, "music_id", music.song_id);
			avs2::property_node_create(dst_prop, node, avs2::NODE_TYPE_s32, "kind", 1);
			avs2::property_node_create(dst_prop, node, avs2::NODE_TYPE_s32, "note_bit", notebit);
			avs2::property_node_create(dst_prop, node, avs2::NODE_TYPE_str, "music_pack_item_id", "");
		}

		auto snd_mgr = iidx::get_snd_mgr();
		snd_mgr->mount_sound(80064, 0);
		return true;
	}

	utils::hook::detour property_node_refer;
	int property_node_refer_hook(avs2::property_ptr prop, avs2::node_ptr node, const char* path, avs2::node_type type, void* data, uint32_t data_size)
	{
		// force enable premium pass
		if (type == avs2::node_type::NODE_TYPE_bool && path == "/pdata/premium_pass"s)
		{
			*reinterpret_cast<char*>(data) = 1;
			return 0;
		}

		return property_node_refer.invoke<int>(prop, node, path, type, data, data_size);
	}

#ifndef STABLE
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

			assert(!strstr(fsroot, "/dl_fs3"));

			if (fstype == "extdrmfs"s)
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

	int on_umount(const char* mountpoint)
	{
		if (mountpoint)
		{
			printf("umount %s\n", mountpoint);
		}

		return avs2::fs_umount(mountpoint);
	}
#endif

	void unlock_item(uint8_t* target, const size_t index, const char* item_id, const int free_count, const int non_free_count)
	{
		std::memcpy(target + 28 * index, item_id, 9);
		utils::hook::set<int>(target + 28 * index + 0x14, free_count);
		utils::hook::set<int>(target + 28 * index + 0x18, non_free_count);

		printf("unlocked %s\n", item_id);
	}

	int item_list_export_struct(uint8_t* target)
	{
		const auto& game_module = game::environment::get_module();
		
		static auto items_pattern = game_module.match_sig("89 44 24 58 48 8D 3D ? ? ? ?");
		static auto items = reinterpret_cast<char**>(items_pattern + 11 + *reinterpret_cast<uint32_t*>(items_pattern + 7));

		int count = 0;
		for (int i = 0; ; i++)
		{
			auto item = items[i];

			// ticket & ldisc
			if (item[2] == '0')
				unlock_item(target, count++, item, 9999, 9999);
			else
				unlock_item(target, count++, item, 0, 1);

			if (item == "I2199999"s)
				break;
		}

		utils::hook::set(target + 28 * count, count);
		printf("unlocked %d items\n", count);

		return 0;
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			const auto& game_module = game::environment::get_module();

			// disable signature check
			auto* signature_check_loc = game_module.match_sig("48 8B D6 48 8B CB FF 90 80");
			assert(signature_check_loc);
			utils::hook::nop(signature_check_loc + 6, 8);
			utils::hook::set<uint8_t>(signature_check_loc + 14, 0xEB);

			// disable music list checksum check
			auto music_list_checksum_loc = game_module.match_sig("75 5E 48 8B D6 E8");
			assert(music_list_checksum_loc);
			utils::hook::nop(music_list_checksum_loc, 14);

			// generate music list from client
			auto music_list_import_loc = game_module.match_sig("FF 15 ? ? ? ? 48 85 C0 75 34");
			assert(music_list_import_loc);
			utils::hook::call(music_list_import_loc, generate_music_list);

			// change server url
			auto get_service_url_loc = game_module.match_sig("84 D2 74 16 45");
			assert(get_service_url_loc);
			utils::hook::jump(get_service_url_loc, get_service_url);
			printf("Using bootstrap url: %s\n", get_service_url(nullptr, false, false));

			// unlock all items
			auto item_list_export_struct_loc = game_module.match_sig("75 1C 48 8D 0D ? ? ? ? E8");
			assert(item_list_export_struct_loc);
			utils::hook::call(item_list_export_struct_loc + 9, item_list_export_struct);

			// override asio device name
			if (game::environment::get_param("IIDX_SOUND_MODE") == "1")
			{
				auto device_name = game::environment::get_param("IIDX_ASIO_DEVICE");
				if (device_name.size() > 0x2047)
					throw std::exception("ASIO Device Name is too long!");

				std::memcpy(asio_name, device_name.data(), device_name.size());

				auto asio_device_loc = game_module.match_sig("44 0F B7 8D ? ? ? ? 44");
				assert(asio_device_loc);
				utils::hook::inject(asio_device_loc + 18, asio_name);
				printf("I:launcher: using asio device: %s\n", asio_name);

				// enable retry logic for asio
				auto retry_logic_jnz_loc = game_module.match_sig("45 85 FF 0F 85 93 00 00 00");
				assert(retry_logic_jnz_loc);
				utils::hook::nop(retry_logic_jnz_loc + 3, 6);

				auto retry_logic_sstep_init_call_loc = game_module.match_sig("44 AC 00 00 48 8B CD E8");
				assert(retry_logic_sstep_init_call_loc);
				auto init_superstep_sound_addr = utils::hook::extract<size_t>(retry_logic_sstep_init_call_loc + 8);
				init_superstep_sound_hook.create(init_superstep_sound_addr, init_superstep_sound_stub);
			}

			property_node_refer.create(avs2::property_node_refer.get(), property_node_refer_hook);
		}

		void* load_import(const std::string& library, const std::string& function) override
		{
#ifndef STABLE
			if (library == "avs2-core.dll")
			{
				if (function == "#76")
				{
					return on_mount;
				}
				
				if (function == "#77")
				{
					return on_umount;
				}
				
			}
#endif


			return nullptr;
		}
	};
}

REGISTER_COMPONENT(iidx::patches::component, launcher::game::iidx)
