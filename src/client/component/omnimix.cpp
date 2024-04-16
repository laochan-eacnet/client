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

		auto omni_file = game::avs_fs_open("/data/omni_data.json", 1, 420);
		if (omni_file < 0) {
			printf("E:omnimix: can not load /data/omni_data.json\n");
			return result;
		}

		game::avs_stat stat = { 0 };
		game::avs_fs_fstat(omni_file, &stat);

		if (stat.filesize <= 0) {
			printf("E:omnimix: can not load /data/omni_data.json\n");
			return result;
		}

		std::string omni_json;
		omni_json.resize(stat.filesize + 1);

		game::avs_fs_read(omni_file, omni_json.data(), stat.filesize);
		game::avs_fs_close(omni_file);
		simdjson::ondemand::parser parser;
		//simdjson::padded_string padded_json(omni_json);
		simdjson::ondemand::document omni_data = parser.iterate(omni_json);
		auto test = omni_data.at(0);
		uint64_t id = test["id"].get_uint64();//这一步就爆炸了 但是新开的项目没问题
		//const json omni_data = json::parse(omni_json);

		//if (!omni_data.is_array()) {
		//	printf("E:omnimix: omni_data is not array!\n");
		//	return result;
		//}

		for (simdjson::ondemand::object item : parser.iterate(omni_json))
		{
			//if (!item.is_object()) {
			//	printf("W:omnimix: music item is not object!\n");
			//	continue;
			//}

			const auto id = item["id"].get_uint64().take_value();
			const auto index = music_data->index_table[id];

			if ((id >= CUR_STYLE_ENTRIES && index == 0) || index == 0xffff)
				continue;
			
			auto* const music = music_data->musics + index;

			auto bpms = item["bpms"];
			auto bpmsp = bpms["sp"];
			auto bpmdp = bpms["dp"];
			auto count = item["noteCounts"];
			auto spcount = count["sp"];
			auto dpcount = count["dp"];

			for (size_t i = 0; i < 5; i++)
			{
				auto currentspbpm = bpmsp.at(i);
				auto currentdpbpm = bpmdp.at(i);
				music->bpm[i].min = currentspbpm["min"].get_uint64();
				music->bpm[i].max = currentspbpm["max"].get_uint64();
				music->bpm[i + 5].min = currentdpbpm["min"].get_uint64();
				music->bpm[i + 5].max = currentdpbpm["max"].get_uint64();

				music->note_count[i] = spcount.at(i).get_uint64();
				music->note_count[i + 5] = dpcount.at(i).get_uint64();
			}
		}

		return result;
	}

	void insert_music_datas()
	{
		auto omni_file = game::avs_fs_open("/data/omni_data.json", 1, 420);
		if (omni_file < 0) {
			printf("E:omnimix: can not load /data/omni_data.json\n");
			game::finalize_music_data();
			return;
		}

		const auto music_data = game::get_music_data();
		auto backup = utils::memory::allocate<game::music_data_t>(0x400000);
		std::memcpy(backup, music_data, 0x400000);

		game::avs_stat stat = { 0 };
		game::avs_fs_fstat(omni_file, &stat);
		if (stat.filesize <= 0) {
			printf("E:omnimix: can not load /data/omni_data.json\n");
			game::finalize_music_data();
			return;
		}

		std::string omni_json;
		omni_json.resize(stat.filesize + 1);

		game::avs_fs_read(omni_file, omni_json.data(), stat.filesize);
		game::avs_fs_close(omni_file);

		const json omni_data = json::parse(omni_json);
		std::vector<game::music_t> musics;

		if (!omni_data.is_array()) {
			printf("E:omnimix: omni_data is not array!\n");
			goto exit;
		}

		for (const auto& item : omni_data)
		{
			if (!item.is_object()) {
				printf("W:omnimix: music item is not object!\n");
				continue;
			}

			game::music_t music = { 0 };

#define LOAD_STRING(field, path) \
	{ \
		const auto str = item##path.get<std::string>(); \
		const auto shiftjis_str = utils::string::wide_to_shiftjis(utils::string::utf8_to_wide(str)); \
		memcpy(music.##field, shiftjis_str.data(), std::min(sizeof(music.##field), shiftjis_str.size())); \
	}

#define LOAD_INT(field, path) \
	music.##field = item##path.get<uint32_t>()
#define LOAD_SHORT(field, path) \
	music.##field = item##path.get<uint16_t>()
#define LOAD_BYTE(field, path) \
	music.##field = item##path.get<uint8_t>()

			LOAD_INT(song_id, ["id"]);

			LOAD_STRING(title, ["title"]);
			LOAD_STRING(title_ascii, ["englishTitle"]);
			LOAD_STRING(genre, ["genre"]);
			LOAD_STRING(artist, ["artist"]);

			LOAD_INT(texture_title, ["textures"]["title"]);
			LOAD_INT(texture_artist, ["textures"]["artist"]);
			LOAD_INT(texture_genre, ["textures"]["genre"]);
			LOAD_INT(texture_load, ["textures"]["load"]);
			LOAD_INT(texture_list, ["textures"]["list"]);

			LOAD_INT(font_idx, ["fontId"]);
			LOAD_SHORT(game_version, ["version"]);

			LOAD_SHORT(other_folder, ["otherFolder"]);
			LOAD_SHORT(bemani_folder, ["bemaniFolder"]);
			LOAD_SHORT(splittable_diff, ["splitTableDiff"]);

			for (int i = 0; i < 5; i++)
			{
				LOAD_BYTE(level_sp[i], ["level"]["sp"][i]);
				LOAD_BYTE(level_dp[i], ["level"]["dp"][i]);
				LOAD_BYTE(ident_sp[i], ["ident"]["sp"][i]);
				LOAD_BYTE(ident_dp[i], ["ident"]["dp"][i]);
			}

			LOAD_INT(song_id, ["id"]);
			LOAD_INT(volume, ["volume"]);

			LOAD_SHORT(bga_delay, ["bgaDelay"]);
			LOAD_STRING(bga_filename, ["bgaFile"]);

			LOAD_INT(afp_flag, ["afpFlag"]);

			if (!item.contains("afpData"))
			{
				musics.push_back(music);
				continue;
			}

			if (!item["afpData"].is_array())
			{
				printf("W:omnimix: afp_data is not array.\n");
				continue;
			}

			for (size_t n = 0; n < 10; n++)
			{
				auto hex = item["afpData"][n].get<std::string>();

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
				music_data->index_table[i] = 0xffffi16;
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
