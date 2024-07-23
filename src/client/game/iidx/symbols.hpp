#pragma once

namespace iidx
{
	using namespace game;

	WEAK symbol<void()> finalize_music_data{ 0x1400D9790 };
	WEAK symbol<music_data_t* ()> get_music_data{ 0x1401C4120 };
	WEAK symbol<LRESULT(HWND, UINT, WPARAM, LPARAM)> main_wndproc{ 0x140202100 };

	WEAK vftable_entry<bool(void*)> music_select_scene_attach{ 0x1404D9610, 13 };
	WEAK vftable_entry<void(void*)> music_select_scene_detach{ 0x1404D9610, 14 };

	WEAK vftable_entry<bool(void*)> dan_select_flow_attach{ 0x1404CC1B8, 13 };
	WEAK vftable_entry<void(void*)> dan_select_flow_detach{ 0x1404CC1B8, 14 };

	WEAK vftable_entry<void(StageResultDrawFrame_s*, void*)> stage_result_draw_frame_init{ 0x1404CB160, 1 };

	namespace EacnetRequestPost
	{
		WEAK symbol<bool(eacnet_request_post_s*, void*, void*)> OnRequestPropertyExported{ 0x140302E30 };
	}

	WEAK symbol<IDirect3DDevice9Ex*> d3d9ex_device{ 0x142AAD3F8 };
	WEAK symbol<IDirect3DDevice9*>  d3d9_device{ 0x142AAD3F8 };
	WEAK symbol<HWND*> main_hwnd{ 0x142AAD3E8 };
	WEAK symbol<const char> infinitas_id{ 0x142AAD388 };
	WEAK symbol<const char> game_version{ 0x141AC54E3 };

	WEAK symbol<bool> show_options{ 0x141ACE104 };
	WEAK symbol<bool> show_consume_window{ 0x1405EFA60 };
	WEAK symbol<int> selected_gauge_type{ 0x141ACE138 };

	WEAK symbol<state_t> state{ 0x1426A15A4 };
}
