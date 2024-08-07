#pragma once

namespace iidx
{
	using namespace game;

	WEAK symbol<void()> finalize_music_data{ 0x1400D97C0 };
	WEAK symbol<music_data_t* ()> get_music_data{ 0x1401C42D0 };
	WEAK symbol<LRESULT(HWND, UINT, WPARAM, LPARAM)> main_wndproc{ 0x1402022B0 };

	WEAK vftable_entry<bool(void*)> music_select_scene_attach{ 0x1404D9658, 13, launcher::game::iidx };
	WEAK vftable_entry<void(void*)> music_select_scene_detach{ 0x1404D9658, 14, launcher::game::iidx };

	WEAK vftable_entry<bool(void*)> dan_select_scene_attach{ 0x1404CC1F8, 13, launcher::game::iidx };
	WEAK vftable_entry<void(void*)> dan_select_scene_detach{ 0x1404CC1F8, 14, launcher::game::iidx };

	WEAK vftable_entry<void(StageResultDrawFrame_s*, void*)> stage_result_draw_frame_init{ 0x1404CB188, 1, launcher::game::iidx };

	namespace EacnetRequestPost
	{
		WEAK symbol<bool(eacnet_request_post_s*, void*, void*)> OnRequestPropertyExported{ 0x140302FE0 };
	}

	WEAK symbol<IDirect3DDevice9Ex*> d3d9ex_device{ 0x142AAD410 };
	WEAK symbol<IDirect3DDevice9*> d3d9_device{ 0x142AAD410 };
	WEAK symbol<HWND*> main_hwnd{ 0x142AAD400 };
	WEAK symbol<const char> infinitas_id{ 0x141AC5494 };
	WEAK symbol<const char> game_version{ 0x141AC54E3 };

	WEAK symbol<bool> show_options{ 0x141ACE00F };
	WEAK symbol<bool> show_consume_window{ 0x1405EFAA0 };
	WEAK symbol<int> selected_gauge_type{ 0x141ACE128 };

	WEAK symbol<state_t> state{ 0x1426A15D4 };
}
