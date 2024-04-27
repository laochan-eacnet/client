#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/memory.hpp>
#include <utils/string.hpp>
#include <utils/image.hpp>
#include <utils/hook.hpp>
#include <game/game.hpp>

#include <component/filesystem.hpp>
#include "component/steam_proxy.hpp"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx9.h>

using json = nlohmann::json;

#define RAD(n) (n * static_cast<float>(std::numbers::pi) / 180.f)

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace overlay
{
	bool is_imgui_inited = false;
	bool is_destoryed = false;
	bool shutting_down = false;
	char* font_data;
	std::chrono::steady_clock::time_point last_frame;
	std::vector<float> frametimes;

	ImFont* font_normal;
	ImFont* font_big;

	bool show_information = false;
	bool show_clock = false;
	bool show_extra_music_info = true;

	game::CMusicSelectScene_s* music_select_scene;

	ImTextureID load_texture(const std::string& path)
	{
		filesystem::file texture_file{ path };

		if (!texture_file.exists())
		{
			return nullptr;
		}

		auto* const device = *game::d3d9ex_device;

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

		struct radar_data
		{
			uint16_t notes;
			uint16_t peak;
			uint16_t scratch;
			uint16_t soflan;
			uint16_t charge;
			uint16_t chord;
		};

		std::map<int, std::vector<radar_data>> note_radars;

		void init()
		{
			textures::background = load_texture("/data/notes_radar/nradar_waku02.png");
			textures::star = load_texture("/data/notes_radar/radar_star.png");
			textures::labels[4] = load_texture("/data/notes_radar/nradar_notes.png");
			textures::labels[5] = load_texture("/data/notes_radar/nradar_peak.png");
			textures::labels[0] = load_texture("/data/notes_radar/nradar_scratch.png");
			textures::labels[1] = load_texture("/data/notes_radar/nradar_sof_lan.png");
			textures::labels[2] = load_texture("/data/notes_radar/nradar_charge.png");
			textures::labels[3] = load_texture("/data/notes_radar/nradar_chord.png");
			textures::no_data = load_texture("/data/notes_radar/nradar_nodata.png");

			filesystem::file radar_file{ "/data/radar_data.json" };
			if (!radar_file.exists()) {
				printf("E:overlay: can not load /data/radar_data.json\n");
				return;
			}

			json data = json::parse(radar_file.get_buffer());

			if (!data.is_array()) {
				printf("E:overlay: radar_data is not array\n");
				return;
			}

			for (const auto& music : data) {
				const auto id = music["id"].get<int>();
				std::vector<radar_data> radars;

				for (const auto& radar : music["radars"])
				{
					radars.push_back({
						radar["notes"].get<uint16_t>(),
						radar["peak"].get<uint16_t>(),
						radar["scratch"].get<uint16_t>(),
						radar["soflan"].get<uint16_t>(),
						radar["charge"].get<uint16_t>(),
						radar["chord"].get<uint16_t>(),
						});
				}

				note_radars.emplace(id, radars);
			}
		}

		void add_image_center(ImDrawList* const draw_list, ImTextureID image, ImVec2 pos, int width, int height)
		{
			draw_list->AddImage(image, ImVec2(pos.x - (width / 2), pos.y - (height / 2)), ImVec2(pos.x + (width / 2), pos.y + (height / 2)));
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

			if (*game::show_options || *game::show_consume_window)
				return;

			auto* const draw_list = ImGui::GetForegroundDrawList();
			add_image_center(draw_list, textures::background, center, 182, 182);

			const auto music_id = music_select_scene->current_music->song_id;
			const auto chart = music_select_scene->selected_chart;

			auto iter = note_radars.find(music_id);
			if (iter == note_radars.end())
			{
				add_image_center(draw_list, textures::no_data, center, 116, 21);
				return;
			}

			const auto data = iter->second[chart];

			if (!data.scratch && !data.soflan && !data.charge && !data.chord && !data.notes && !data.peak)
			{
				add_image_center(draw_list, textures::no_data, center, 116, 21);
				return;
			}

			float size = 78.f;

			float sizes[] = { data.scratch / 10000.f, data.soflan / 10000.f, data.charge / 10000.f, data.chord / 10000.f, data.notes / 10000.f, data.peak / 10000.f };
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


				draw_list->PathLineTo(ImVec2(center.x + x1, center.y + y1));
				draw_list->PathLineTo(ImVec2(center.x + x2, center.y + y2));
				draw_list->PathLineTo(center);

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


					draw_list->AddText(ImVec2(center.x + x, center.y + y + 2), colors[i], disp_value);
					draw_list->AddText(ImVec2(center.x + x, center.y + y), 0xFFFFFFFF, disp_value);
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
			filesystem::file diff_file{ "/data/difficulty_data.json" };
			if (!diff_file.exists()) {
				printf("E:overlay: can not load /data/difficulty_data.json\n");
				return;
			}

			json data = json::parse(diff_file.get_buffer());

			if (!data.is_array()) {
				printf("E:overlay: difficulty_data is not array\n");
				return;
			}

			for (const auto& table : data)
			{
				difficulty_table t;
				t.name = table["name"].get<std::string>();
				t.musics = std::map<int, std::map<int, note_difficulty_data>>{};

				if (!table["musics"].is_array()) {
					printf("E:overlay: table.musics is not array\n");
					continue;
				}

				for (const auto& music : table["musics"])
				{
					std::map<int, note_difficulty_data> notes;

					if (!music["notes"].is_array()) {
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

			if (*game::show_options || *game::show_consume_window)
				return;

			const auto music_id = music_select_scene->current_music->song_id;
			const auto chart = music_select_scene->selected_chart;

			difficulty_table* table;

			if (show_aaa_table)
			{
				table = &aaa_difficulty_table;
			}
			else
			{
				auto iter = gauge_difficulty_tables.find(*game::selected_gauge_type);
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

			ImGui::SetNextWindowPos(ImVec2(980, 500));
			ImGui::SetNextWindowSize(ImVec2(200, 145));

			if (ImGui::Begin("DIFFICULTY", nullptr,
				ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_NoSavedSettings
			)) {
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

	static void init_imgui()
	{
		IMGUI_CHECKVERSION();

		ImGui::CreateContext();

		const auto& io = ImGui::GetIO();
		const auto font_file = game::avs_fs_open("/data/font/DF-PopMix-W5.ttc", 1, 420);
		game::avs_stat state{ 0 };
		game::avs_fs_fstat(font_file, &state);

		font_data = utils::memory::allocate<char>(state.filesize);

		game::avs_fs_read(font_file, font_data, state.filesize);
		game::avs_fs_close(font_file);

		font_normal = io.Fonts->AddFontFromMemoryTTF(
			font_data, state.filesize, 16.0f,
			nullptr, io.Fonts->GetGlyphRangesJapanese()
		);

		font_big = io.Fonts->AddFontFromMemoryTTF(
			font_data, state.filesize, 24.0f,
			nullptr, io.Fonts->GetGlyphRangesJapanese()
		);

		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(*game::main_hwnd);
		ImGui_ImplDX9_Init(*game::d3d9_device);

		notes_radar::init();
		notes_difficulty::init();

		is_imgui_inited = true;
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
		)) {

			ImGui::Text(utils::string::va(VERSION " (%s)", game::game_version));
			ImGui::Text(utils::string::va("ID: %s", game::infinitas_id));
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
		)) {
			ImGui::PlotLines("",
				frametimes.data(),
				static_cast<int>(frametimes.size()),
				0, nullptr, 0.f, max + 10.f,
				ImVec2(300, 75)
			);

			ImGui::SetCursorPosY(7);

			ImGui::Text(utils::string::va(VERSION " (%s)", game::game_version));
			ImGui::Text(utils::string::va("ID: %s", game::infinitas_id));
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
		)) {

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
			show_information = !show_information;

		// F11 - toggle clock
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F11), false))
			show_clock = !show_clock;

		// F10 - toggle notes radar, etc
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F10), false))
			show_extra_music_info = !show_extra_music_info;

		// F9 - toggle diff table type
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F9), false))
			notes_difficulty::show_aaa_table = !notes_difficulty::show_aaa_table;

		if (show_information)
			draw_debug_information();
		else
			draw_watermark();

		if (show_clock)
			draw_clock();

		if (show_extra_music_info)
		{
			notes_radar::draw();
			notes_difficulty::draw();
		}
	}

	static LRESULT wndproc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
	{
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, w_param, l_param))
			return true;

		return game::main_wndproc(hwnd, msg, w_param, l_param);
	}

	bool music_select_scene_attach(game::CMusicSelectScene_s* scene)
	{
		printf("D:overlay: CMusicSelectScene::OnAttach\n");

		steam_proxy::set_status("\xF0\x9F\x97\xBF IIDX - SELECT MUSIC");

		music_select_scene = scene;
		return game::music_select_scene_attach(scene);
	}

	const char* chart_names[] = {
		"SPB", "SPN", "SPH", "SPA", "SPL",
		"DPB", "DPN", "DPH", "DPA", "DPL",
	};

	void music_select_scene_detach(game::CMusicSelectScene_s* scene)
	{
		printf("D:overlay: CMusicSelectScene::OnDetach\n");

		music_select_scene = nullptr;
		return game::music_select_scene_detach(scene);
	}

	utils::hook::detour base_stage_attach;
	bool base_stage_attach_hook(void* scene)
	{
		printf("D:overlay: CBaseStageScene::OnAttach\n");

		bool result = base_stage_attach.invoke<bool>(scene);

		if (!game::state->music)
			return result;

		auto music = game::state->music;
		auto dj = game::state->p1_active ? 0 : 1;
		auto chart = game::state->chart[dj];

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

	void stage_result_draw_frame_init(game::StageResultDrawFrame_s *_this, int* unk1)
	{
		game::stage_result_draw_frame_init(_this, unk1);

		if (!game::state->music)
			return;

		auto dj = game::state->p1_active ? 0 : 1;

		auto clear_type = clear_names[_this->player[dj].clear_type];
		auto djlevel = dj_level_names[_this->player[dj].dj_level];

		auto status = utils::string::va("\xF0\x9F\x97\xBF IIDX - STAGE RESULT: %s DJ LEVEL: %s", clear_type, djlevel);
		steam_proxy::set_status(status);
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

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();

		// HACK: this game uses fixed 1080p backbuffer
		auto& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(1920, 1080);

		ImGui::NewFrame();

		frametimes.push_back(io.DeltaTime * 1000);

		if (frametimes.size() > 60)
			frametimes.erase(frametimes.begin());

		draw_gui();

		ImGui::EndFrame();

		auto* const device = *game::d3d9ex_device;

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
		void post_start() override
		{
			present.create(0x1401F75B0, present_stub);

			// hook wndproc
			utils::hook::inject(0x140202272, wndproc);

			utils::hook::set(0x1404D8630, music_select_scene_attach);
			utils::hook::set(0x1404D8638, music_select_scene_detach);

			utils::hook::set(0x1404CA128, stage_result_draw_frame_init);
			base_stage_attach.create(0x140184D30, base_stage_attach_hook);
		}

		void pre_destroy() override
		{
			if (!is_imgui_inited)
				return;

			shutting_down = true;
		}
	};
}

REGISTER_COMPONENT(overlay::component)
