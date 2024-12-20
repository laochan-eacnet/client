#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/memory.hpp>
#include <game/game.hpp>

#include "component/filesystem.hpp"
#include "analyze.hpp"

using json = nlohmann::json;

namespace iidx::omnimix
{
	struct muisc_32_t
	{
		wchar_t title[0x80];
		char title_ascii[0x40];
		wchar_t genre[0x40];
		wchar_t artist[0x80];

		uint8_t gap1[0x100];

		struct
		{
			uint32_t title;
			uint32_t artist;
			uint32_t genre;
			uint32_t load;
			uint32_t list;
			uint32_t unk;
		} textures;

		uint32_t font_id;
		uint16_t version;

		struct
		{
			uint16_t other;
			uint16_t bemani;
			uint16_t unk0;
			uint16_t unk1;
			uint16_t unk2;
			uint16_t split_table_diff;
			uint16_t unk3;
		} folders;

		uint8_t level[10];

		uint8_t gap2[0x286];

		uint32_t id;
		uint32_t volume;

		uint8_t ident[10];
		uint16_t bga_delay;
		char bga_file[0x20];
		uint32_t afp_flag;
		struct
		{
			uint8_t data[0x20];
		} afp_data[10];
		int unk2;
	};

	struct music_data_32_t
	{
		char magic[4];
		uint32_t data_version;
		uint32_t count;
		uint32_t max_entries;
		uint32_t index[(32 + 1) * 1000];
		muisc_32_t musics[1];
	};

	utils::hook::detour get_name_hook;
	const char* get_bga_name(iidx::music_t* music, int note_id)
	{
		const auto music_bga = music->bga_filename;
		auto path = utils::string::va("/ac_mount/movie/%s.mp4", music_bga);

		if (filesystem::exists(path))
		{
			return utils::string::va("%s.mp4", music_bga);
		}

		path = utils::string::va("/ac_mount/movie/%s.wmv", music_bga);

		if (filesystem::exists(path))
		{
			return utils::string::va("%s.wmv", music_bga);
		}

		return get_name_hook.invoke<const char*, iidx::music_t*, int>(music, note_id);
	}

	csd_t* csd_load_hook(csd_t* result, const char* name)
	{
		result->pad = 0;

		int folder;
		sscanf_s(name, "%d/%d.%*s", &folder, &result->id);
		snprintf(result->path, 256, "/ac_mount/sound/%s", name);

		if (filesystem::exists(result->path))
			return result;

		snprintf(result->path, 256, "/data/sound/%s", name);

		if (filesystem::exists(result->path))
			return result;

		result->id = 0;

		printf("libutil: CSDLoad::CSDLoad sound file not found: %s\n", name);
		return result;
	}

	int get_movie_path(char* buffer, size_t buffer_size, const char*, const char* movie_name)
	{
		auto c = snprintf(buffer, buffer_size, "/ac_mount/movie/%s", movie_name);
		if (filesystem::exists(buffer))
			return c;

		return snprintf(buffer, buffer_size, "/data/movie/%s", movie_name);
	}

	int get_layer_path(char* buffer, size_t buffer_size, const char*, const char* layer_name)
	{
		if ((*layer_name == 'i' && *(layer_name + 1) == '_') ||
			(*layer_name >= '0' && *layer_name <= '9'))
		{
			auto c = snprintf(buffer, buffer_size, "/ac_mount/graphic/%s", layer_name);
			if (filesystem::exists(buffer))
				return c;
		}

		return snprintf(buffer, buffer_size, "/data/graphic/%s", layer_name);
	}

	std::vector<json> get_additional_mdatas()
	{
		auto dir = avs2::fs_opendir("/laochan/music_datas");

		std::vector<json> result;

		if (dir <= 0)
			return result;

		for (auto file = avs2::fs_readdir(dir); file; file = avs2::fs_readdir(dir))
		{
			filesystem::file mdata_file{ utils::string::va("/laochan/music_datas/%s", file) };
			if (!mdata_file.exists())
				continue;

			result.push_back(json::parse(mdata_file.get_buffer()));
		}

		return result;
	}

	utils::hook::detour load_music_info_hook;
	uint64_t load_music_info()
	{
		auto result = load_music_info_hook.invoke<uint64_t>();
		const auto music_data = iidx::get_music_data();

		for (size_t i = 0; i < music_data->music_count; i++)
		{
			auto& music = music_data->musics[i];

			bool had_data = false;
			for (size_t j = 0; j < 10; j++)
			{
				if (music.note_count[j])
				{
					had_data = true;
					break;
				}
			}

			if (had_data)
				continue;

			auto id = music.song_id;

			csd_t csd;
			csd_load(&csd, utils::string::va("%d/%d.1", id, id));

			if (!csd.id)
				continue;

			filesystem::file chart_file{ csd.path };
			auto& data = chart_file.get_buffer();

			for (int j = 0; j < 10; j++)
			{
				auto mapped = analyze::map_chart(j);

				auto offset = *reinterpret_cast<const uint32_t*>(data.data() + mapped * 8);
				auto size = *reinterpret_cast<const uint32_t*>(data.data() + mapped * 8 + 4);

				if (!offset || !size) continue;

				std::vector<event_t> events;
				events.resize(size / sizeof(event_t));
				std::memcpy(events.data(), data.data() + offset, size);

				analyze::chart_analyze_data_t analyze_data;
				analyze::analyze_chart(events, analyze_data, false);

				music.bpm[j].min = static_cast<uint32_t>(analyze_data.bpm.min);
				music.bpm[j].max = static_cast<uint32_t>(analyze_data.bpm.max);
				music.note_count[j] = static_cast<uint32_t>(analyze_data.note_count);
			}
		}

		return result;
	}

	void insert_music_datas()
	{
		filesystem::file ac_music_data{ "/ac_mount/info/0/music_data.bin" };

		const music_data_32_t* ac_data = nullptr;

		if (!ac_music_data.exists())
		{
			printf("E:omnimix: can not load ac music_data.bin\n");
		}
		else
		{
			auto& buffer = ac_music_data.get_buffer();
			ac_data = reinterpret_cast<const music_data_32_t*>(buffer.data());
		}

		const auto music_data = iidx::get_music_data();
		auto backup = utils::memory::allocate<iidx::music_data_t>(0x400000);
		std::memcpy(backup, music_data, 0x400000);

		auto _ = gsl::finally([=] {
			iidx::finalize_music_data();
			utils::memory::free(backup);
		});

		auto mdatas = get_additional_mdatas();

		std::vector<iidx::music_t> musics;

		if (ac_data)
		{
			for (size_t i = 0; i < ac_data->count; i++)
			{
				iidx::music_t inf_music{};
				auto& ac_music = ac_data->musics[i];

				auto title = utils::string::wide_to_shiftjis(ac_music.title);
				std::memcpy(inf_music.title, title.data(), std::min<size_t>(0x40, title.size()));

				std::memcpy(inf_music.title_ascii, ac_music.title_ascii, 0x40);

				auto genre = utils::string::wide_to_shiftjis(ac_music.genre);
				std::memcpy(inf_music.genre, genre.data(), std::min<size_t>(0x40, genre.size()));

				auto artist = utils::string::wide_to_shiftjis(ac_music.artist);
				std::memcpy(inf_music.artist, artist.data(), std::min<size_t>(0x40, artist.size()));

				// no textures as mdata.ifs is not handled right now (maybe never)
				inf_music.texture_title = 0;
				inf_music.texture_artist = 0;
				inf_music.texture_genre = 0;
				inf_music.texture_load = 0;
				inf_music.texture_title = 0;

				inf_music.font_idx = ac_music.font_id;

				// we have no iidx32 folder in infinitas now
				inf_music.game_version = ac_music.version == 32 ? 80 : ac_music.version;

				inf_music.other_folder = ac_music.folders.other;
				inf_music.bemani_folder = ac_music.folders.bemani;
				inf_music.splittable_diff = ac_music.folders.split_table_diff;

				std::memcpy(inf_music.level, ac_music.level, 10);

				inf_music.song_id = ac_music.id;
				inf_music.volume = ac_music.volume;

				std::memcpy(inf_music.ident_sp, ac_music.ident, 10);

				inf_music.bga_delay = ac_music.bga_delay;
				std::memcpy(inf_music.bga_filename, ac_music.bga_file, 0x20);

				inf_music.afp_flag = ac_music.afp_flag;
				std::memcpy(inf_music.afp_data, ac_music.afp_data, 0x20 * 10);

				musics.push_back(inf_music);
			}
		}

		for (const auto& item : mdatas)
		{
			if (!item.is_object())
			{
				printf("W:omnimix: music item is not object!\n");
				continue;
			}

			iidx::music_t music = { 0 };

#define LOAD_STRING(field, path) \
	{ \
		const std::string str = item##path.get<std::string>(); \
		const auto shiftjis_str = utils::string::wide_to_shiftjis(utils::string::utf8_to_wide(str)); \
		memcpy(music.##field, shiftjis_str.data(), std::min(sizeof(music.##field), shiftjis_str.size())); \
	}

#define LOAD_INT(field, path) \
	music.##field = item##path.get<uint32_t>();
#define LOAD_SHORT(field, path) \
	music.##field =item##path.get<uint16_t>();
#define LOAD_BYTE(field, path) \
	music.##field = item##path.get<uint8_t>();

			LOAD_INT(song_id, ["id"]);

			// skip existing song
			if (backup->index_table[music.song_id] > 0)
			{
				continue;
			}

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
				LOAD_BYTE(level[i], ["level"]["sp"].at(i));
				LOAD_BYTE(level[i + 5], ["level"]["dp"].at(i));
				LOAD_BYTE(ident_sp[i], ["ident"]["sp"].at(i));
				LOAD_BYTE(ident_dp[i], ["ident"]["dp"].at(i));
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
				std::string hex = item["afpData"][n].get<std::string>();

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
		{
			const auto find_result = std::find_if(musics.begin(), musics.end(), [=](const iidx::music_t& v) {
				return v.song_id == backup->musics[i].song_id;
			});

			if (find_result == musics.end())
				musics.push_back(backup->musics[i]);
		}

		printf("I:omnimix: total music count: %llu\n", musics.size());

		std::sort(musics.begin(), musics.end(), [](const iidx::music_t& a, const iidx::music_t& b) -> bool {
			return a.song_id < b.song_id;
		});

		music_data->music_count = static_cast<uint16_t>(musics.size());

		for (size_t i = 0; i < MAX_ENTRIES; i++)
		{
			const auto find_result = std::find_if(musics.begin(), musics.end(), [i](const iidx::music_t& v) {
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
			std::memcpy(music_data->musics + i, &musics[i], sizeof(iidx::music_t));
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			// return success if file not exists
			utils::hook::set<uint16_t>(0x1401F16B8, 0x01B0);

			// allow mp4 and wmv bga
			get_name_hook.create(0x1401C4530, get_bga_name);

			// remove music count limit
			utils::hook::set<uint8_t>(0x1401C336E, 0xEB);

			// add omni songs to music_data.bin
			utils::hook::call(0x1401C337E, insert_music_datas);

			// load omni song detail
			load_music_info_hook.create(0x1401C33C0, load_music_info);

			// load ac files if ac file exists
			utils::hook::jump(iidx::csd_load.get(), csd_load_hook);
			utils::hook::call(0x14020C870, get_movie_path);
			utils::hook::call(0x14020CE7D, get_movie_path);
			utils::hook::call(0x140105A9D, get_layer_path);
			utils::hook::call(0x140174610, get_layer_path);
			utils::hook::call(0x140224386, get_layer_path);
			utils::hook::call(0x140224636, get_layer_path);
		}

		void post_avs_init() override
		{
			avs2::fs_mount("/ac_mount", "E:\\LDJ-012-2023101800\\contents\\data", "fs", const_cast<char*>("vf=1,posix=1"));
		}
	};
}

REGISTER_COMPONENT(iidx::omnimix::component, launcher::game::iidx)
