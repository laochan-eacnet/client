#pragma once

namespace iidx
{
	using namespace game;

	WEAK symbol<bool(const char* args)> game_preinit{ 0x1401BBD50 };
	WEAK symbol<bool()> init_avs{ 0x1400C6410 };
	WEAK symbol<void()> finalize_music_data{ 0x1400D98E0 };
	WEAK symbol<music_data_t* ()> get_music_data{ 0x1401C3FC0 };
	WEAK symbol<LRESULT(HWND, UINT, WPARAM, LPARAM)> main_wndproc{ 0x140201FA0 };

	WEAK symbol<bool(void*)> music_select_scene_attach{ 0x140144220 };
	WEAK symbol<void(void*)> music_select_scene_detach{ 0x140144780 };

	WEAK symbol<bool(void*)> dan_select_flow_attach{ 0x14011F4D0 };
	WEAK symbol<void(void*)> dan_select_flow_detach{ 0x14011F500 };

	WEAK symbol<void(StageResultDrawFrame_s*, void*)> stage_result_draw_frame_init{ 0x140175E10 };

	namespace EacnetRequestPost
	{
		WEAK symbol<bool(eacnet_request_post_s*, void*, void*)> OnRequestPropertyExported{ 0x140302CD0 };
	}

	WEAK symbol<void*(size_t size)> malloc{ 0x1403DEA10 };
	WEAK symbol<void(void* block)> free{ 0x1403DE9D0 };

	WEAK symbol<IDirect3DDevice9Ex*> d3d9ex_device{ 0x142AAC410 };
	WEAK symbol<IDirect3DDevice9*>  d3d9_device{ 0x142AAC410 };
	WEAK symbol<HWND*> main_hwnd{ 0x142AAC400 };
	WEAK symbol<const char> infinitas_id{ 0x141AC4494 };
	WEAK symbol<const char> game_version{ 0x141AC44E3 };

	WEAK symbol<bool> show_options{ 0x141ACD104 };
	WEAK symbol<bool> show_consume_window{ 0x1405EE970 };
	WEAK symbol<int> selected_gauge_type{ 0x141ACD138 };

	WEAK symbol<state_t> state{ 0x1426A05C4 };
}
