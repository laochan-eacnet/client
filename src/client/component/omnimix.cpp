#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/memory.hpp>
#include <game/game.hpp>

using json = nlohmann::json;

namespace omnimix
{
	utils::hook::detour get_name_hook;
	const char* get_bga_name(game::music_t* music, int note_id)
	{
		const auto music_bga = music->bga_filename;
		auto path = utils::string::va("/data/movie/%s.mp4", music_bga);
		game::avs_stat stat = { 0 };
		game::avs_fs_lstat(path, &stat);

		if (stat.filesize > 0) {
			return utils::string::va("%s.mp4", music_bga);
		}

		path = utils::string::va("/data/movie/%s.wmv", music_bga);
		game::avs_fs_lstat(path, &stat);

		if (stat.filesize > 0) {
			return utils::string::va("%s.wmv", music_bga);
		}

		return get_name_hook.invoke<const char*, game::music_t*, int>(music, note_id);
	}

	utils::hook::detour load_music_info_hook;
	uint64_t load_music_info()
	{
		auto result = load_music_info_hook.invoke<uint64_t>();
		const auto music_data = game::get_music_data();



		return result;
	}

	void insert_music_datas()
	{
		auto omni_file = game::avs_fs_open("/data/omni_musics.json", 1, 420);
		if (omni_file < 0) {
			printf("E:omnimix: can not load /data/omni_musics.json\n");
			game::finalize_music_data();
			return;
		}

		const auto music_data = game::get_music_data();
		auto backup = utils::memory::allocate<game::music_data_t>(0x400000);
		std::memcpy(backup, music_data, 0x400000);

		game::avs_stat stat = { 0 };
		game::avs_fs_fstat(omni_file, &stat);

		std::string omni_json;
		omni_json.resize(stat.filesize + 1);

		game::avs_fs_read(omni_file, omni_json.data(), stat.filesize);
		game::avs_fs_close(omni_file);

		const json omni_data = json::parse(omni_json);
		std::vector<game::music_t> musics;

		if (!omni_data.is_array()) {
			printf("E:omnimix: omni_musics is not array!\n");
			goto exit;
		}

		for (const auto& item : omni_data)
		{
			if (!item.is_object()) {
				printf("W:omnimix: music item is not object!\n");
				continue;
			}

			game::music_t music = { 0 };

#define LOAD_STRING(field) \
	{ \
		const auto str = item[#field].get<std::string>(); \
		const auto shiftjis_str = utils::string::wide_to_shiftjis(utils::string::utf8_to_wide(str)); \
		strncpy_s(music.##field, shiftjis_str.data(), sizeof(music.##field)); \
	}

	
#define LOAD_INT(field) \
	music.##field = item[#field].get<uint32_t>()
#define LOAD_SHORT(field) \
	music.##field = item[#field].get<uint16_t>()
#define LOAD_BYTE(field) \
	music.##field = item[#field].get<uint8_t>()

			music.song_id = item["song_id"].get<uint32_t>();

			LOAD_STRING(title);
			LOAD_STRING(title_ascii);
			LOAD_STRING(genre);
			LOAD_STRING(artist);
			
			LOAD_INT(texture_title);
			LOAD_INT(texture_artist);
			LOAD_INT(texture_genre);
			LOAD_INT(texture_load);
			LOAD_INT(texture_list);

			LOAD_INT(font_idx);
			LOAD_SHORT(game_version);

			LOAD_SHORT(other_folder);
			LOAD_SHORT(bemani_folder);
			LOAD_SHORT(splittable_diff);

			LOAD_BYTE(SPB_level);
			LOAD_BYTE(SPN_level);
			LOAD_BYTE(SPH_level);
			LOAD_BYTE(SPA_level);
			LOAD_BYTE(SPL_level);

			LOAD_BYTE(DPB_level);
			LOAD_BYTE(DPN_level);
			LOAD_BYTE(DPH_level);
			LOAD_BYTE(DPA_level);
			LOAD_BYTE(DPL_level);

			LOAD_INT(song_id);
			LOAD_INT(volume);

			LOAD_BYTE(SPB_ident);
			LOAD_BYTE(SPN_ident);
			LOAD_BYTE(SPH_ident);
			LOAD_BYTE(SPA_ident);
			LOAD_BYTE(SPL_ident);

			LOAD_BYTE(DPB_ident);
			LOAD_BYTE(DPN_ident);
			LOAD_BYTE(DPH_ident);
			LOAD_BYTE(DPA_ident);
			LOAD_BYTE(DPL_ident);

			LOAD_SHORT(bga_delay);
			LOAD_STRING(bga_filename);

			LOAD_INT(afp_flag);

			if (!item["afp_data"].is_array())
			{
				printf("W:omnimix: afp_data is not array.\n");
				continue;
			}

			for (size_t n = 0; n < 10; n++)
			{
				auto hex = item["afp_data"][n].get<std::string>();
				
				for (size_t i = 0; i < 0x20; i++)
				{
					const auto& byte = hex.substr(i * 2, 2);
					music.afp_data[n * 0x20 + i] = static_cast<uint8_t>(std::strtoul(byte.data(), nullptr, 16));
				}
			}

			musics.push_back(music);
		}

		printf("I:omnimix: parsed %llu musics\n", musics.size());

		for (size_t i = 0; i < backup->music_count; i++)
			musics.push_back(backup->musics[i]);

		printf("I:omnimix: total music count: %llu\n", musics.size());
		
		std::sort(musics.begin(), musics.end(), [](const game::music_t& a, const game::music_t& b) -> bool {
			return a.song_id < b.song_id;
		});

		music_data->music_count = static_cast<uint16_t>(musics.size());
		
		for (size_t i = 0; i < MAX_ENTRIES; i++)
		{
			const auto find_result = std::find_if(musics.begin(), musics.end(), [i](const game::music_t& v) {
				return v.song_id == i;
			});

			if (find_result != musics.end())
			{
				music_data->index_table[i] = static_cast<uint16_t>(find_result - musics.begin());
				continue;
			}

			if (i >= CUR_STYLE_ENTRIES)
				music_data->index_table[i] = 0;
			else 
				music_data->index_table[i] = 0xffff;
		}

		for (size_t i = 0; i < music_data->music_count; i++)
			std::memcpy(music_data->musics + i, &musics[i], sizeof(game::music_t));

	exit:
		utils::memory::free(backup);
		game::finalize_music_data();
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			// return success if file not exists
			utils::hook::set<uint16_t>(0x1401E49E8, 0x01B0);

			// allow mp4 and wmv bga
			get_name_hook.create(0x1401B7910, get_bga_name);

			// add omni songs to music_data.bin
			utils::hook::call(0x1401B675E, insert_music_datas);

			// load omni song detail
			load_music_info_hook.create(0x1401B67A0, load_music_info);
		}
	};
}

REGISTER_COMPONENT(omnimix::component)
