#include <std_include.hpp>
#include <cstdarg>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/memory.hpp>
#include <game/game.hpp>

#include "component/filesystem.hpp"
#include "analyze.hpp"
#include "env.hpp"

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
		if (env::use_exp())
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
		}
		else
		{
			const auto music_bga = music->bga_filename;
			auto path = utils::string::va("/data/movie/%s.mp4", music_bga);

			if (filesystem::exists(path))
			{
				return utils::string::va("%s.mp4", music_bga);
			}

			path = utils::string::va("/data/movie/%s.wmv", music_bga);

			if (filesystem::exists(path))
			{
				return utils::string::va("%s.wmv", music_bga);
			}
		}

		return get_name_hook.invoke<const char*, iidx::music_t*, int>(music, note_id);
	}

	void mount_sound_ifs(const char* src, int32_t id)
	{
		std::string mountpoint_lnk = utils::string::va("/dl_sound_lnk/sound/%d.ifs", id);
		avs2::fs_mount(mountpoint_lnk.data(), src, "link", nullptr);

		std::string mountpoint = utils::string::va("/data/sound/%d", id);
		avs2::fs_mount(mountpoint.data(), mountpoint_lnk.data(), "imagefs", nullptr);
	}

	csd_t* csd_load_hook(csd_t* result, const char* name)
	{
		int folder = 0;
		result->pad = 0;
		snprintf(result->path, 256, "/data/sound/%s", name);

		if (filesystem::exists(result->path))
		{
			return result;
		}

		char suffix[5];
		sscanf_s(name, "%d/%d.%4s", &folder, &result->id, suffix, static_cast<int>(_countof(suffix)));
		auto ifs = utils::string::va("/dl_fs3/sound/%05d.ifs", result->id);

		if (filesystem::exists(ifs))
		{
			mount_sound_ifs(ifs, result->id);

			snprintf(result->path, 256, "/data/sound/%s", name);

			if (filesystem::exists(result->path))
				return result;
		}

		// fix the god damn file name
		if (strstr(name, "preview"))
		{
			sscanf_s(name, "preview/%d_pre.2dx", &result->id);
			name = utils::string::va("%05d/%05d_pre.2dx", result->id, result->id);
		}
		else if (result->id < 10000)
		{
			name = utils::string::va("%05d/%05d.%s", result->id, result->id, suffix);
		}

		snprintf(result->path, 256, "/ac_mount/sound/%s", name);

		if (filesystem::exists(result->path))
			return result;

		snprintf(result->path, 256, "/sd%05d/%s", result->id, name);

		if (filesystem::exists(result->path))
			return result;

		ifs = utils::string::va("/ac_mount/sound/%05d-p0.ifs", result->id);

		if (!filesystem::exists(ifs))
			ifs = utils::string::va("/ac_mount/sound/%05d.ifs", result->id);

		if (filesystem::exists(ifs))
		{
			mount_sound_ifs(ifs, result->id);
			snprintf(result->path, 256, "/sd%05d/%s", result->id, name);

			if (filesystem::exists(result->path))
				return result;
		}

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

	int avs2_snprintf_hook(char* buffer, size_t buffer_size, const char* fmt, ...)
	{
		std::va_list va;
		va_start(va, fmt);

		int result = 0;

		if (fmt == "/data/movie/%s"s)
		{
			result = get_movie_path(buffer, buffer_size, nullptr, va_arg(va, const char*));
		}
		else if (fmt == "/data/graphic/%s"s)
		{
			result = get_layer_path(buffer, buffer_size, nullptr, va_arg(va, const char*));
		}
		else
		{
			result = vsnprintf(buffer, buffer_size, fmt, va);
		}

		va_end(va);

		return result;
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

	utils::hook::detour insert_music_datas_hook;
	void insert_music_datas()
	{
		filesystem::file ac_music_data{ env::use_exp() ? "/ac_mount/info/0/music_data.bin" : "/data/omni_musics.bin" };

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
			{
				musics.push_back(backup->musics[i]);
				continue;
			}

			int chart_count_inf = 0;
			int chart_count_ac = 0;

			for (size_t j = 0; j < 10; j++)
			{
				if (find_result->level[i]) chart_count_ac++;
				if (backup->musics[i].level[i]) chart_count_inf++;
			}

			if (chart_count_inf >= chart_count_ac)
			{
				musics.erase(find_result);
				musics.push_back(backup->musics[i]);
			}
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

		return insert_music_datas_hook.invoke<void>();
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			const auto& game_module = game::environment::get_module();

			// return success if file not exists
			auto file_exists_patch_loc = game_module.match_sig("32 C0 E9 2B 03 00 00");
			assert(file_exists_patch_loc);
			utils::hook::set<uint16_t>(file_exists_patch_loc, 0x01B0);

			// allow mp4 and wmv bga
			auto get_bga_name_loc = game_module.match_sig("3D A3 42 00 00 75 12");
			assert(get_bga_name_loc);
			get_name_hook.create(get_bga_name_loc - 24, get_bga_name);

			// remove music count limit
			// NOTE: actually not required...but we use this as signature for next patch
			auto music_count_limit_loc = game_module.match_sig("41 81 F9 D0 07");
			assert(music_count_limit_loc);
			utils::hook::set<uint8_t>(music_count_limit_loc + 7, 0xEB);

			// add omni songs to music_data.bin
			auto insert_music_data_call_loc = utils::hook::follow_branch(music_count_limit_loc + 7);
			auto insert_music_data_loc = utils::hook::extract<void*>(insert_music_data_call_loc + 1);
			insert_music_datas_hook.create(insert_music_data_loc, insert_music_datas);

			// load ac files if ac file exists
			utils::hook::jump(iidx::csd_load.get(), csd_load_hook);
		}

		void post_avs_init() override
		{
			if (!env::use_exp())
				return;

			auto acdata_path = game::environment::get_param("IIDX_ACDATA_PATH");
			if (!std::filesystem::exists(acdata_path))
				return;

			avs2::fs_mount("/ac_mount", acdata_path.data(), "fs", nullptr);
		}

		void* load_import(const std::string& library, const std::string& function) override
		{
			// load ac files if ac file exists
			if (env::use_exp() && library == "avs2-core.dll" && function == "#258")
			{
				return avs2_snprintf_hook;
			}

			return nullptr;
		}
	};
}

#ifndef STABLE
REGISTER_COMPONENT(iidx::omnimix::component, launcher::game::iidx)
#endif
