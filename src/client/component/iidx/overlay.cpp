﻿#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/memory.hpp>
#include <utils/string.hpp>
#include <utils/image.hpp>
#include <utils/hook.hpp>
#include <game/game.hpp>

#include "component/filesystem.hpp"
#include "component/steam_proxy.hpp"

#include "custom_resolution.hpp"
#include "chart_modifier.hpp"
#include "analyze.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx9.h>

#include <windowsx.h>

using json = nlohmann::json;

#define RAD(n) (n * static_cast<float>(std::numbers::pi) / 180.f)

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace iidx::overlay
{
	bool is_imgui_inited = false;
	bool is_destoryed = false;
	bool shutting_down = false;
	bool is_block_input = false;
	char* font_data;
	std::chrono::steady_clock::time_point last_frame;
	std::vector<float> frametimes;

	ImFont* font_normal;
	ImFont* font_big;

	int show_information = 1;
	bool show_clock = false;
	bool show_extra_music_info = true;

	bool in_game = false;

	iidx::CMusicSelectScene_s* music_select_scene;
	void* dan_stage_flow;

	ImVec2 scale(ImVec2 input)
	{
		static auto x_scale = custom_resolution::width() / 1920.f;
		static auto y_scale = custom_resolution::height() / 1080.f;

		input.x = input.x * x_scale;
		input.y = input.y * y_scale;

		return input;
	}

	void show_cursor(bool show)
	{
		static bool last = false;

		if (last != show) ShowCursor(show);
		last = show;
	}

	ImTextureID load_texture(const std::string& path)
	{
		filesystem::file texture_file{ path };

		if (!texture_file.exists())
		{
			return nullptr;
		}

		auto* const device = *iidx::d3d9ex_device;

		utils::image texture_image{ texture_file.get_buffer() };

		LPDIRECT3DTEXTURE9 result;
		if (device->CreateTexture(texture_image.get_width(), texture_image.get_height(), 0, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &result, nullptr))
		{
			return nullptr;
		}

		D3DLOCKED_RECT tex_locked_rect;
		if (result->LockRect(0u, &tex_locked_rect, nullptr, D3DLOCK_DISCARD))
		{
			return nullptr;
		}

		auto buffer = static_cast<const uint32_t*>(texture_image.get_buffer());
		auto target_buffer = static_cast<uint32_t*>(tex_locked_rect.pBits);
		const auto w = texture_image.get_width();
		const auto h = texture_image.get_height();

		// ABGR to ARGB
		for (size_t y = 0; y < h; y++) for (size_t x = 0; x < w; x++)
		{
			auto b = (buffer[x + y * w] & 0x00FF0000) >> 16;
			auto r = buffer[x + y * w] & 0x000000FF;

			target_buffer[x + tex_locked_rect.Pitch / sizeof(uint32_t) * y] = (buffer[x + y * w] & 0xFF00FF00) | (r << 16) | b;
		}

		result->UnlockRect(0);
		return result;
	}

	namespace notes_radar
	{
		namespace textures
		{
			ImTextureID background, star, no_data;
			ImTextureID labels[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		}

		float angles[] = { RAD(30), RAD(90), RAD(150), RAD(210), RAD(270), RAD(330) };
		float label_dist[] = { 110, 95, 110, 110, 95, 110 };
		ImColor colors[] = { ImColor(255,0,0,200), ImColor(0,150,255,200), ImColor(88,15,247,200), ImColor(127,224,0,200), ImColor(255,64,235,200), ImColor(255,108,0,200) };

		std::map<int, std::vector<analyze::chart_analyze_data_t>> analyze_datas;

		analyze::chart_analyze_data_t* get_analyze_data(int id, size_t diff)
		{
			auto iter = analyze_datas.find(id);
			if (iter != analyze_datas.end())
				return &iter->second[diff];

			csd_t csd;
			csd_load(&csd, utils::string::va("%d/%d.1", id, id));

			if (!csd.id)
			{
				return nullptr;
			}

			filesystem::file chart_file{ csd.path };

			if (!chart_file.exists())
			{
				return nullptr;
			}

			auto& data = chart_file.get_buffer();
			std::vector<analyze::chart_analyze_data_t> datas;

			for (int i = 0; i < 10; i++)
			{
				auto m = analyze::map_chart(i);

				auto offset = *reinterpret_cast<const uint32_t*>(data.data() + m * 8);
				auto size = *reinterpret_cast<const uint32_t*>(data.data() + m * 8 + 4);

				analyze::chart_analyze_data_t analyze_data;
				memset(&analyze_data, 0, sizeof(analyze::chart_analyze_data_t));

				if (offset && size)
				{
					std::vector<event_t> events;
					events.resize(size / sizeof(event_t));
					std::memcpy(events.data(), data.data() + offset, size);

					analyze::analyze_chart(events, analyze_data, true);
				}

				datas.push_back(analyze_data);
			}

			analyze_datas.emplace(id, datas);
			return &analyze_datas[id][diff];
		}

		void init()
		{
			textures::background = load_texture("/laochan/notes_radar/nradar_waku02.png");
			textures::star = load_texture("/laochan/notes_radar/radar_star.png");
			textures::labels[4] = load_texture("/laochan/notes_radar/nradar_notes.png");
			textures::labels[5] = load_texture("/laochan/notes_radar/nradar_peak.png");
			textures::labels[0] = load_texture("/laochan/notes_radar/nradar_scratch.png");
			textures::labels[1] = load_texture("/laochan/notes_radar/nradar_sof_lan.png");
			textures::labels[2] = load_texture("/laochan/notes_radar/nradar_charge.png");
			textures::labels[3] = load_texture("/laochan/notes_radar/nradar_chord.png");
			textures::no_data = load_texture("/laochan/notes_radar/nradar_nodata.png");
		}

		void add_image_center(ImDrawList* const draw_list, ImTextureID image, ImVec2 pos, int width, int height)
		{
			draw_list->AddImage(image, scale(ImVec2(pos.x - (width / 2), pos.y - (height / 2))), scale(ImVec2(pos.x + (width / 2), pos.y + (height / 2))));
		}

		ImVec2 center = ImVec2(1080, 360);
		float draw_sizes[] = { 0, 0, 0, 0, 0, 0 };
		ImColor draw_color = colors[0];

		void draw()
		{
			if (!music_select_scene || !music_select_scene->current_music)
				return;

			if (music_select_scene->music_decide_layer && music_select_scene->decide_music)
				return;

			if (*iidx::show_options || *iidx::show_consume_window)
				return;

			auto* const draw_list = ImGui::GetForegroundDrawList();
			add_image_center(draw_list, textures::background, center, 182, 182);

			const auto music_id = music_select_scene->current_music->song_id;
			auto chart = iidx::state->p1_active ? music_select_scene->selected_chart_p1 : music_select_scene->selected_chart_p2;

			auto data = get_analyze_data(music_id, chart);

			if (!data)
			{
				add_image_center(draw_list, textures::no_data, center, 116, 21);
				return;
			}

			music_select_scene->current_music->bpm[chart].min = static_cast<uint32_t>(data->bpm.min);
			music_select_scene->current_music->bpm[chart].max = static_cast<uint32_t>(data->bpm.max);
			music_select_scene->current_music->note_count[chart] = static_cast<uint32_t>(data->note_count);

			float size = 78.f;

			float sizes[] = { data->radar.scratch, data->radar.soflan, data->radar.charge, data->radar.chord, data->radar.notes, data->radar.peak };
			size_t star_index = 0;

			for (size_t i = 0; i < 6; i++)
			{
				draw_sizes[i] = draw_sizes[i] / 2 + sizes[i] / 2;

				if (sizes[i] > sizes[star_index])
					star_index = i;
			}

			draw_color.Value.x = draw_color.Value.x / 2 + colors[star_index].Value.x / 2;
			draw_color.Value.y = draw_color.Value.y / 2 + colors[star_index].Value.y / 2;
			draw_color.Value.z = draw_color.Value.z / 2 + colors[star_index].Value.z / 2;

			for (size_t i = 0; i < 6; i++)
			{
				const auto idx = (i + 1) % 6;

				auto x1 = draw_sizes[i] * size * std::cos(angles[i]);
				auto y1 = draw_sizes[i] * size * std::sin(angles[i]);

				auto x2 = draw_sizes[idx] * size * std::cos(angles[idx]);
				auto y2 = draw_sizes[idx] * size * std::sin(angles[idx]);


				draw_list->PathLineTo(scale(ImVec2(center.x + x1, center.y + y1)));
				draw_list->PathLineTo(scale(ImVec2(center.x + x2, center.y + y2)));
				draw_list->PathLineTo(scale(center));

				draw_list->PathFillConcave(draw_color);
				draw_list->PathClear();
			}

			for (size_t i = 0; i < 6; i++)
			{
				{
					auto x = label_dist[i] * std::cos(angles[i]);
					auto y = label_dist[i] * std::sin(angles[i]);

					add_image_center(draw_list, textures::labels[i], ImVec2(center.x + x, center.y + y), 74, 12);

					if (i == star_index)
					{
						add_image_center(draw_list, textures::star, ImVec2(center.x + x, center.y + y), 110, 22);
					}
				}

				{
					auto disp_value = utils::string::va("%.2f", draw_sizes[i] * 100);
					auto text_size = ImGui::CalcTextSize(disp_value);


					auto x = label_dist[i] * std::cos(angles[i]) - text_size.x * 0.5f;
					auto y = label_dist[i] * std::sin(angles[i]) + (i < 3 ? 8 : -(8 + text_size.y));


					draw_list->AddText(scale(ImVec2(center.x + x, center.y + y + 2)), colors[i], disp_value);
					draw_list->AddText(scale(ImVec2(center.x + x, center.y + y)), 0xFFFFFFFF, disp_value);
				}

			}
		}
	}

	namespace notes_difficulty
	{
		struct note_difficulty_data
		{
			std::string difficulty;
			std::string description;
		};

		struct difficulty_table
		{
			std::string name;
			std::map<int, std::map<int, note_difficulty_data>> musics;
		};

		std::map<int, difficulty_table> gauge_difficulty_tables;
		difficulty_table aaa_difficulty_table;

		bool show_aaa_table = false;

		void init()
		{
			filesystem::file diff_file{ "/laochan/difficulty_data.json" };
			if (!diff_file.exists())
			{
				printf("E:overlay: can not load /laochan/difficulty_data.json\n");
				return;
			}

			json data = json::parse(diff_file.get_buffer());

			if (!data.is_array())
			{
				printf("E:overlay: difficulty_data is not array\n");
				return;
			}

			for (const auto& table : data)
			{
				difficulty_table t;
				t.name = table["name"].get<std::string>();
				t.musics = std::map<int, std::map<int, note_difficulty_data>>{};

				if (!table["musics"].is_array())
				{
					printf("E:overlay: table.musics is not array\n");
					continue;
				}

				for (const auto& music : table["musics"])
				{
					std::map<int, note_difficulty_data> notes;

					if (!music["notes"].is_array())
					{
						printf("E:overlay: table.musics.notes is not array\n");
						continue;
					}

					for (const auto& note : music["notes"])
					{
						note_difficulty_data d;
						d.difficulty = note["difficulty"].get<std::string>();
						d.description = note["description"].get<std::string>();

						notes.emplace(note["note"].get<int>(), d);
					}

					t.musics.emplace(music["id"].get<int>(), notes);
				}

				auto table_type = table["tableType"].get<int>();

				if (table_type == 0)
				{
					auto gauge_type = table["gaugeType"].get<int>();
					gauge_difficulty_tables.emplace(gauge_type, t);
				}
				else if (table_type == 1)
				{
					aaa_difficulty_table = t;
				}
			}
		}

		void draw()
		{
			if (!music_select_scene || !music_select_scene->current_music)
				return;

			if (music_select_scene->music_decide_layer && music_select_scene->decide_music)
				return;

			if (*iidx::show_options || *iidx::show_consume_window)
				return;

			const auto music_id = music_select_scene->current_music->song_id;
			auto chart = iidx::state->p1_active ? music_select_scene->selected_chart_p1 : music_select_scene->selected_chart_p2;

			difficulty_table* table;

			if (show_aaa_table)
			{
				table = &aaa_difficulty_table;
			}
			else
			{
				auto iter = gauge_difficulty_tables.find(iidx::option->selected_gauge_type);
				if (iter == gauge_difficulty_tables.end())
					return;

				table = &iter->second;
			}

			auto music_iter = table->musics.find(music_id);
			if (music_iter == table->musics.end())
				return;

			auto music = &music_iter->second;

			auto note_iter = music->find(chart);
			if (note_iter == music->end())
				return;

			auto note = note_iter->second;

			ImGui::SetNextWindowPos(scale(ImVec2(980, 500)));
			ImGui::SetNextWindowSize(scale(ImVec2(200, 145)));

			if (ImGui::Begin("DIFFICULTY", nullptr,
							 ImGuiWindowFlags_NoDecoration |
							 ImGuiWindowFlags_NoSavedSettings
			))
			{
				ImGui::Text(table->name.data());

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);

				ImGui::Text(reinterpret_cast<const char*>(u8"推定難易度:"));

				ImGui::PushFont(font_big);
				ImGui::Text(note.difficulty.data());
				ImGui::PopFont();

				ImGui::Text(note.description.data());

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);

				ImGui::Text(reinterpret_cast<const char*>(u8"類型切換 F9"));

				ImGui::End();
			}
		}
	}

	namespace modifier_selector
	{
		std::vector<uint64_t> random_template;
		int random_template_item = 0;
		static char random_template_search_text[8] = { 0 };

		void draw()
		{
			if (!music_select_scene && !dan_stage_flow)
				return;

			if (!*iidx::show_options)
				return;

			show_cursor(true);

			uint32_t modifier = chart_modifier::get();
			static bool open_random_window = false;

			if (iidx::state->p1_active)
				ImGui::SetNextWindowPos(scale(ImVec2(460, 975)));
			else
				ImGui::SetNextWindowPos(scale(ImVec2(840, 975)));

			if (ImGui::Begin("MODIFIER", nullptr,
							 ImGuiWindowFlags_NoDecoration |
							 ImGuiWindowFlags_NoSavedSettings |
							 ImGuiWindowFlags_NoBackground))
			{
				ImGui::CheckboxFlags(" ALL SCRATCH ", &modifier, chart_modifier::all_scratch);
				ImGui::SameLine();
				ImGui::CheckboxFlags(" ALL CHARGE ", &modifier, chart_modifier::all_charge);

				ImGui::CheckboxFlags(" D4DJ ", &modifier, chart_modifier::d4dj);

				ImGui::CheckboxFlags(" REGULAR SPEED ", &modifier, chart_modifier::regular_speed);
				ImGui::SameLine();

				ImGui::CheckboxFlags(" FIXED RANDOM ", &modifier, chart_modifier::fixed_random);
				ImGui::SameLine();

				if (ImGui::Button("F-RAN OPTION"))
					open_random_window = true;

				ImGui::End();
			}

			chart_modifier::set(static_cast<chart_modifier::modifier_t>(modifier));

			is_block_input = open_random_window;

			if (!open_random_window)
				return;

			ImGui::PushFont(font_big);

			ImGui::SetNextWindowSize(scale(ImVec2(500, 300)));
			ImGui::SetNextWindowPos(scale(ImVec2(710, 390)));

			if (ImGui::Begin("F-RAN Option", nullptr,
							 ImGuiWindowFlags_NoSavedSettings |
							 ImGuiWindowFlags_NoMove |
							 ImGuiWindowFlags_NoResize |
							 ImGuiWindowFlags_NoCollapse))
			{
				static auto _ = []() -> bool {
					std::vector<int8_t> v = { '1', '2', '3', '4', '5', '6', '7' };

					do
					{
						uint64_t t = 0;

						for (size_t i = 0; i < 7; i++)
							t |= static_cast<uint64_t>(v[i]) << i * 8;

						random_template.push_back(t);
					} while (std::next_permutation(v.begin(), v.end()));

					return true;
				}();

				ImGui::Text("SEARCH:");
				if (ImGui::InputText("##", random_template_search_text, 8))
				{
					random_template.clear();
					random_template_item = 0;

					std::vector<int8_t> v = { '1', '2', '3', '4', '5', '6', '7' };

					do
					{
						bool is_target = true;
						uint64_t t = 0;

						for (size_t i = 0; i < 7; i++)
						{
							if (random_template_search_text[i] == 0)
								break;

							if (v[i] != random_template_search_text[i])
							{
								is_target = false;
								break;
							}
						}

						if (!is_target)
							continue;

						for (size_t i = 0; i < 7; i++)
							t |= static_cast<uint64_t>(v[i]) << i * 8;

						random_template.push_back(t);
					} while (std::next_permutation(v.begin(), v.end()));
				}

				ImGui::ListBox("##", &random_template_item, [](void*, int index) -> const char* {
					return reinterpret_cast<const char*>(random_template.data() + index);
				}, nullptr, static_cast<int>(random_template.size()), 5);

				if (ImGui::Button("USE SELECTED"))
				{
					auto t = reinterpret_cast<int8_t*>(random_template.data() + random_template_item);
					int8_t new_template[7];

					for (size_t i = 0; i < 7; i++)
						new_template[i] = t[i] - '1';

					chart_modifier::set_fran(new_template);
					open_random_window = false;
				}

				ImGui::End();
			}
			ImGui::PopFont();
		}
	}

	static void init_imgui()
	{
		IMGUI_CHECKVERSION();

		ImGui::CreateContext();

		const auto& io = ImGui::GetIO();
		const auto font_file = filesystem::file{ "/data/font/DF-PopMix-W5.ttc" };
		auto font_buffer = font_file.get_buffer();

		font_data = utils::memory::allocate<char>(font_buffer.size());
		std::memcpy(font_data, font_buffer.data(), font_buffer.size());

		auto ratio = (custom_resolution::width() * 1.f) / custom_resolution::height();
		auto scale_factor = custom_resolution::height() / 1080.f;

		if (ratio < (16.f / 9.f))
			scale_factor = custom_resolution::width() / 1920.f;

		font_normal = io.Fonts->AddFontFromMemoryTTF(
			font_data, static_cast<int>(font_buffer.size()), 16.0f * scale_factor,
			nullptr, io.Fonts->GetGlyphRangesJapanese()
		);

		font_big = io.Fonts->AddFontFromMemoryTTF(
			font_data, static_cast<int>(font_buffer.size()), 24.0f * scale_factor,
			nullptr, io.Fonts->GetGlyphRangesJapanese()
		);

		ImGui::StyleColorsDark();
		auto& style = ImGui::GetStyle();
		style.ScaleAllSizes(scale_factor);

		ImGui_ImplWin32_Init(*iidx::main_hwnd);
		ImGui_ImplDX9_Init(*iidx::d3d9ex_device);

		notes_radar::init();
		notes_difficulty::init();

		is_imgui_inited = true;
	}

	static const char* game_version()
	{
		const static auto ver = ([] {
			return game::environment::get_version();
		})();

		return ver.data();
	}

	static void draw_watermark()
	{
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 20));

		ImGui::SetNextWindowPos(ImVec2(5, 5));
		if (ImGui::Begin("INFORMATION", nullptr,
						 ImGuiWindowFlags_NoInputs |
						 ImGuiWindowFlags_NoDecoration |
						 ImGuiWindowFlags_NoNav |
						 ImGuiWindowFlags_NoBackground |
						 ImGuiWindowFlags_NoSavedSettings |
						 ImGuiWindowFlags_AlwaysAutoResize
		))
		{

			ImGui::Text(utils::string::va(VERSION " (%s)", game_version()));
			ImGui::Text(utils::string::va("ID: %s", iidx::infinitas_id.get()));
			ImGui::End();
		}

		ImGui::PopStyleColor();
	}

	static void draw_debug_information()
	{
		float last_frametime = 0;
		float frametime_sum = 0;
		float max = 33.3f;

		for (const auto ft : frametimes)
		{
			last_frametime = ft;
			frametime_sum += ft;

			if (ft > max) max = ft;
		}

		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 190));
		ImGui::PushStyleColor(ImGuiCol_PlotLines, IM_COL32(255, 255, 255, 120));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 0, 0, 90));

		ImGui::SetNextWindowPos(ImVec2(5, 5));

		if (ImGui::Begin("INFORMATION", nullptr,
						 ImGuiWindowFlags_NoInputs |
						 ImGuiWindowFlags_NoDecoration |
						 ImGuiWindowFlags_NoNav |
						 ImGuiWindowFlags_NoBackground |
						 ImGuiWindowFlags_NoSavedSettings |
						 ImGuiWindowFlags_AlwaysAutoResize
		))
		{
			ImGui::PlotLines("",
							 frametimes.data(),
							 static_cast<int>(frametimes.size()),
							 0, nullptr, 0.f, max + 10.f,
							 ImVec2(300, 75)
			);

			ImGui::SetCursorPosY(7);

			ImGui::Text(utils::string::va(VERSION " (%s)", game_version()));
			ImGui::Text(utils::string::va("ID: %s", iidx::infinitas_id.get()));
			ImGui::Text(utils::string::va("FPS: %.1f (%.2fms)", 1000 / (frametime_sum / frametimes.size()), last_frametime));
			ImGui::End();
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}

	static void draw_clock()
	{
		const auto& io = ImGui::GetIO();

		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 190));

		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 5, 5), 0, ImVec2(1, 0));
		if (ImGui::Begin("CLOCK", nullptr,
						 ImGuiWindowFlags_NoInputs |
						 ImGuiWindowFlags_NoDecoration |
						 ImGuiWindowFlags_NoNav |
						 ImGuiWindowFlags_NoBackground |
						 ImGuiWindowFlags_NoSavedSettings |
						 ImGuiWindowFlags_AlwaysAutoResize
		))
		{

			ImGui::PushFont(font_big);
			ImGui::Text(
				std::format("{:%T}",
							std::chrono::current_zone()->to_local(
								std::chrono::time_point_cast<std::chrono::seconds>(
									std::chrono::system_clock::now()
								)
							)
				).data()
			);
			ImGui::PopFont();

			ImGui::End();
		}

		ImGui::PopStyleColor();
	}


	static void draw_gui()
	{
		// F12 - toggle debug info
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F12), false))
			show_information = (show_information + 1) % 3;

		// F11 - toggle clock
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F11), false))
			show_clock = !show_clock;

#ifndef STABLE
		// F10 - toggle notes radar, etc
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F10), false))
			show_extra_music_info = !show_extra_music_info;
#endif

		// F9 - toggle diff table type
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F9), false))
			notes_difficulty::show_aaa_table = !notes_difficulty::show_aaa_table;

		if (show_information == 0)
			draw_debug_information();
		else if (show_information == 1)
			draw_watermark();

		if (show_clock)
			draw_clock();

#ifndef STABLE
		if (show_extra_music_info)
		{
			notes_radar::draw();
			notes_difficulty::draw();
		}
#endif

		modifier_selector::draw();
	}

	static LRESULT wndproc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
	{
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, w_param, l_param))
			return true;

		if (is_block_input && msg == WM_INPUT)
			return true;

		return iidx::main_wndproc(hwnd, msg, w_param, l_param);
	}

	void* music_select_scene_attach_orig = nullptr;
	bool music_select_scene_attach(iidx::CMusicSelectScene_s* scene)
	{
		printf("D:overlay: CMusicSelectScene::OnAttach\n");

		steam_proxy::set_status("\xF0\x9F\x97\xBF IIDX - SELECT MUSIC");

		music_select_scene = scene;
		return utils::hook::invoke<bool>(music_select_scene_attach_orig, scene);
	}

	const char* chart_names[] = {
		"SPB", "SPN", "SPH", "SPA", "SPL",
		"DPB", "DPN", "DPH", "DPA", "DPL",
	};

	void* music_select_scene_detach_orig = nullptr;
	void music_select_scene_detach(iidx::CMusicSelectScene_s* scene)
	{
		printf("D:overlay: CMusicSelectScene::OnDetach\n");

		music_select_scene = nullptr;
		return utils::hook::invoke<void>(music_select_scene_detach_orig, scene);
	}

	utils::hook::detour base_stage_attach;
	bool base_stage_attach_hook(void* scene)
	{
		in_game = true;

		printf("D:overlay: CBaseStageScene::OnAttach\n");

		bool result = base_stage_attach.invoke<bool>(scene);

		if (!iidx::state->music)
			return result;

		auto music = iidx::state->music;
		auto dj = iidx::state->p1_active ? 0 : 1;
		auto chart = iidx::state->chart[dj];

		auto title = utils::string::wide_to_utf8(utils::string::shiftjis_to_wide(music->title));
		auto chart_name = chart_names[chart];
		auto level = music->level[chart];

		steam_proxy::set_status(utils::string::va("\xF0\x9F\x97\xBF IIDX - %s (%s%d)", title.data(), chart_name, level));

		return result;
	}

	const char* dj_level_names[] = {
		"AAA", "AA", "A", "B", "C", "D", "E", "F"
	};

	const char* clear_names[] = {
		"NO PLAY.", "FAILED.", "A-CLEAR.", "E-CLEAR.", "CLEAR.", "H-CLEAR!", "EXH-CLEAR!!", "FULL-COMBO!!!"
	};

	void* stage_result_draw_frame_init_orig = nullptr;
	void stage_result_draw_frame_init(iidx::StageResultDrawFrame_s* _this, int* unk1)
	{
		in_game = false;

		utils::hook::invoke<void>(stage_result_draw_frame_init_orig, _this, unk1);

		if (!iidx::state->music)
			return;

		auto dj = iidx::state->p1_active ? 0 : 1;

		auto clear_type = clear_names[_this->player[dj].clear_type];
		auto djlevel = dj_level_names[_this->player[dj].dj_level];

		auto status = utils::string::va("\xF0\x9F\x97\xBF IIDX - STAGE RESULT: %s DJ LEVEL: %s", clear_type, djlevel);
		steam_proxy::set_status(status);
	}

	void* dan_stage_scene_attach_orig = nullptr;
	bool dan_stage_scene_attach(void* scene)
	{
		printf("D:overlay: CDanSelectFlow::OnAttach\n");

		dan_stage_flow = scene;
		return utils::hook::invoke<bool>(dan_stage_scene_attach_orig, scene);
	}

	void* dan_stage_scene_detach_orig = nullptr;
	void dan_stage_scene_detach(void* scene)
	{
		printf("D:overlay: CDanSelectFlow::OnDetach\n");

		dan_stage_flow = nullptr;
		return utils::hook::invoke<void>(dan_stage_scene_detach_orig, scene);
	}

	void draw()
	{
		if (is_destoryed)
			return;

		if (shutting_down)
		{
			ImGui_ImplDX9_Shutdown();
			ImGui_ImplWin32_Shutdown();
			ImGui::DestroyContext();

			is_destoryed = true;
			return;
		}

		if (!is_imgui_inited)
			init_imgui();

		if (in_game)
			return;

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();

		auto& io = ImGui::GetIO();

		ImGui::NewFrame();

		frametimes.push_back(io.DeltaTime * 1000);

		if (frametimes.size() > 60)
			frametimes.erase(frametimes.begin());

		show_cursor(false);
		draw_gui();

		ImGui::EndFrame();

		auto* const device = *iidx::d3d9ex_device;

		if (device->BeginScene() >= 0)
		{
			IDirect3DSurface9* backbuffer{};
			device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
			device->SetRenderTarget(0, backbuffer);

			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

			device->EndScene();
		}
	}

	utils::hook::detour present;
	void present_stub()
	{
		draw();
		present.invoke<void>();
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
			present.create(iidx::present, present_stub);

			// hook wndproc
			utils::hook::inject(iidx::main_wndproc.get_rva_pos(), wndproc);

			music_select_scene_attach_orig = iidx::music_select_scene.get<void*>(13);
			music_select_scene_detach_orig = iidx::music_select_scene.get<void*>(14);

			utils::hook::set(iidx::music_select_scene.get_ptr(13), music_select_scene_attach);
			utils::hook::set(iidx::music_select_scene.get_ptr(14), music_select_scene_detach);

			dan_stage_scene_attach_orig = iidx::dan_stage_scene.get<void*>(13);
			dan_stage_scene_detach_orig = iidx::dan_stage_scene.get<void*>(14);

			utils::hook::set(iidx::dan_stage_scene.get_ptr(13), dan_stage_scene_attach);
			utils::hook::set(iidx::dan_stage_scene.get_ptr(14), dan_stage_scene_detach);

			stage_result_draw_frame_init_orig = iidx::stage_result_draw_frame.get<void*>(0);
			utils::hook::set(iidx::stage_result_draw_frame.get_ptr(0), stage_result_draw_frame_init);

			base_stage_attach.create(base_stage_scene.get<void*>(13), base_stage_attach_hook);
		}

		void pre_destroy() override
		{
			if (!is_imgui_inited)
				return;

			shutting_down = true;
		}
	};
}

#ifndef STABLE
REGISTER_COMPONENT(iidx::overlay::component, launcher::game::iidx)
#endif
