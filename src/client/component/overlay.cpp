#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/memory.hpp>
#include <utils/string.hpp>
#include <utils/hook.hpp>
#include <game/game.hpp>
#include <component/scheduler.hpp>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx9.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace overlay
{
	bool is_imgui_inited = false;
	bool is_destoryed = false;
	char* font_data;
	std::chrono::steady_clock::time_point last_frame;
	std::vector<float> frametimes;

	ImFont* font_normal;
	ImFont* font_big;

	bool show_information = false;

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
			nullptr, io.Fonts->GetGlyphRangesChineseFull()
		);

		font_big = io.Fonts->AddFontFromMemoryTTF(
			font_data, state.filesize, 24.0f,
			nullptr, io.Fonts->GetGlyphRangesChineseFull()
		);

		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(*game::main_hwnd);
		ImGui_ImplDX9_Init(*game::d3d9_device);

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
			ImGui::Text("PRESS F12 TO EXPAND");
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

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		if (ImGui::Begin("FRAMETIME GLYPH", nullptr,
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

			ImGui::End();
		}

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

		if (show_information)
			draw_debug_information();
		else
			draw_watermark();

		draw_clock();
	}

	static LRESULT wndproc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
	{
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, w_param, l_param))
			return true;

		return game::main_wndproc(hwnd, msg, w_param, l_param);
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			// hook wndproc
			utils::hook::inject(0x1401F5822, wndproc);

			scheduler::schedule([]() -> bool
			{
				if (is_destoryed)
					return scheduler::cond_end;

				const auto now = std::chrono::high_resolution_clock::now();
				const auto diff = now - last_frame;

				last_frame = now;
				frametimes.push_back(diff.count() / 1000000.f);

				if (frametimes.size() > 30)
					frametimes.erase(frametimes.begin());

				if (!is_imgui_inited)
					init_imgui();

				ImGui_ImplDX9_NewFrame();
				ImGui_ImplWin32_NewFrame();
					
				// HACK: this game uses fixed 1080p backbuffer
				auto& io = ImGui::GetIO();
				io.DisplaySize = ImVec2(1920, 1080);

				ImGui::NewFrame();

				draw_gui();

				ImGui::EndFrame();

				auto* const device = *game::d3d9ex_device;

				device->SetRenderState(D3DRS_ZENABLE, FALSE);
				device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

				IDirect3DSurface9* backbuffer{};
				device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
				device->SetRenderTarget(0, backbuffer);

				if (device->BeginScene() >= 0)
				{
					ImGui::Render();
					ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

					device->EndScene();
				}

				return scheduler::cond_continue;
			}, scheduler::pipeline::renderer);
		}

		void pre_destroy() override
		{
			if (!is_imgui_inited)
				return;

			scheduler::once([]() 
			{
				ImGui_ImplDX9_Shutdown();
				ImGui_ImplWin32_Shutdown();
				ImGui::DestroyContext();

				is_destoryed = true;
			}, scheduler::pipeline::renderer);
		}
	};
}

REGISTER_COMPONENT(overlay::component)
