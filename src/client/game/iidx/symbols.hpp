#pragma once

namespace iidx
{
	using namespace game;

	WEAK pattern_extract<music_data_t> music_data_{ "41 B8 00 00 40 00 48 8D 3D", 9, launcher::game::iidx };

	inline music_data_t* get_music_data()
	{
		return music_data_.get();
	}

	WEAK pattern_extract<LRESULT(HWND, UINT, WPARAM, LPARAM)> main_wndproc{ "C7 45 C7 03 00 00 00", 10, launcher::game::iidx };

	WEAK pattern<csd_t* (csd_t* result, const char* name)> csd_load{ "90 C7 83 08", -38, launcher::game::iidx };

	WEAK pattern_extract_vftable music_select_scene{ "C6 43 78 01 48 8D 05", 7, launcher::game::iidx, 7 };
	WEAK pattern_extract_vftable dan_stage_scene{ "88 8B 80 00 00 00 48 8D 05", 9, launcher::game::iidx };
	WEAK pattern_extract_vftable base_stage_scene{ "68 89 43 70 48 8D", 7, launcher::game::iidx };

	WEAK pattern_extract_vftable stage_result_draw_frame{ "48 8B F1 48 8D 05", 6, launcher::game::iidx, 2 };

	namespace EacnetRequestPost
	{
		WEAK pattern<bool(eacnet_request_post_s*, void*, void*)> OnRequestPropertyExported{ "48 81 C1 D4 02 00 00", -9, launcher::game::iidx };
	}

	WEAK pattern_extract<IDirect3DDevice9Ex*> d3d9ex_device{ "C1 FD 0A 48 8B 0D", 6, launcher::game::iidx };
	WEAK pattern_extract<HWND> main_hwnd{ "48 89 45 F0 4C 8B 05", 7, launcher::game::iidx };
	WEAK pattern_extract<char> infinitas_id{ "BA 0E 00 00 00 48 8D 0D", 8, launcher::game::iidx, 3 };

	WEAK pattern_extract<bool> show_options{ "41 0F B6 CC 88 0D", 6, launcher::game::iidx };
	WEAK pattern_extract<bool> show_consume_window{ "48 89 1D ? ? ? ? 89 1D ? ? ? ? C7 05", 3, launcher::game::iidx };
	WEAK pattern_extract<option_t> option{ "78 08 89 2D", 4, launcher::game::iidx };

	WEAK pattern_extract<state_t> state{ "75 EE 8B 15 ? ? ? ? 45 33 C0", 4, launcher::game::iidx };

#ifdef DEBUG
	WEAK pattern_extract<bool(float*, int)> test_fps{ "8B 50 38 E8 ? ? ? ? 84 C0", 4, launcher::game::iidx };
#endif

	WEAK pattern_extract<void(event_t*, int)> post_load_chart{ "E8 ? ? ? ? B0 01 EB 0C", 1, launcher::game::iidx };
	WEAK pattern_extract<void(DWORD*, string*, int, long long)> get_option_str{ "49 83 FF 10 49 0F 43 CE", -17, launcher::game::iidx };

	WEAK pattern_extract<uint8_t> language{ "4C 89 25 ? ? ? ? C7 05", 3, launcher::game::iidx };

	WEAK pattern<void()> present{ "33 FF 8B CF 8B", -27, launcher::game::iidx };

	WEAK pattern_extract<MusicSoundResourceManager*> music_sound_resource_mgr{ "48 89 03 48 89 1D ? ? ? ?", 6, launcher::game::iidx };
		
	inline MusicSoundResourceManager* get_snd_mgr()
	{
		return *music_sound_resource_mgr;
	}
}

