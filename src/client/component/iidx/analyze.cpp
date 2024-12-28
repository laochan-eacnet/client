#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/memory.hpp>
#include <game/game.hpp>

#include "analyze.hpp"

namespace iidx::analyze
{
	float notes(const std::vector<iidx::event_t>& chart)
	{
		int notes = 0, total_tick = 0;

		for (const auto& ev : chart)
		{
			if (ev.type == iidx::event_type_t::end_of_song)
			{
				total_tick = ev.tick;
				break;
			}

			if (ev.type != iidx::event_type_t::note_p1 && ev.type != iidx::event_type_t::note_p2)
				continue;

			notes++;
			if (ev.value) notes++;
		}

		return (notes * 1000.f) / total_tick;
	}

	float peak(const std::vector<iidx::event_t>& chart)
	{
		std::vector<size_t> notes_per_seconds;
		size_t notes_max = 0, notes_near_max = 0;

		for (const auto& ev : chart)
		{
			if (ev.type == iidx::event_type_t::end_of_song)
				break;

			if (ev.type != iidx::event_type_t::note_p1 && ev.type != iidx::event_type_t::note_p2)
				continue;

			auto sec = ev.tick / 1000;
			while (notes_per_seconds.size() <= sec)
				notes_per_seconds.push_back(0);

			if (ev.value)
			{
				auto end_sec = (ev.tick + ev.value) / 1000;
				while (notes_per_seconds.size() <= end_sec)
					notes_per_seconds.push_back(0);

				notes_per_seconds[end_sec]++;
				for (auto i = sec; i <= end_sec; i++)
					notes_per_seconds[i]++;
			}

			notes_per_seconds[sec]++;
		}

		for (size_t i = 0; i < notes_per_seconds.size(); i++)
		{
			auto count = notes_per_seconds[i];
			if (count > notes_max)
				notes_max = count;

			if (i <= 0 || i >= notes_per_seconds.size() - 1)
				continue;

			auto count_near = notes_per_seconds[i + 1] + notes_per_seconds[i - 1] + count;

			if (count_near > notes_near_max)
				notes_near_max = count_near;
		}

		return static_cast<float>(notes_near_max + notes_max);
	}

	float scratch(const std::vector<iidx::event_t>& chart)
	{
		float result = 0;
		int last_tick = 0, total_tick = 0;

		for (const auto& ev : chart)
		{
			if (ev.type == iidx::event_type_t::end_of_song)
			{
				total_tick = ev.tick;
				break;
			}

			if (ev.type != iidx::event_type_t::note_p1 && ev.type != iidx::event_type_t::note_p2)
				continue;

			if (ev.param % 10 != 7)
				continue;

			auto diff = std::max(300.0f - (ev.tick - last_tick), 0.f);
			auto f = (diff / 1000.f) + 1.f;

			result += f;

			if (ev.value)
				result += f;

			if (ev.param / 100 == 1)
				result += f;

			last_tick = ev.tick;
		}

		if (total_tick == 0) return 0;

		return (result * 1000.f) / total_tick;
	}

	float chord(const std::vector<iidx::event_t>& chart)
	{
		std::vector<int> chords;
		float result = 0;
		int last_tick = 0, last_lane = 0;

		for (const auto& ev : chart)
		{
			if (ev.type == iidx::event_type_t::end_of_song)
				break;

			if (ev.type != iidx::event_type_t::note_p1 && ev.type != iidx::event_type_t::note_p2)
				continue;

			auto lane = ev.param % 10;

			if (ev.tick == last_tick)
			{
				chords.push_back(lane);
				continue;
			}

			if (chords.size() > 1)
			{
				result += std::powf(static_cast<float>(chords.size() - 1), 1.2f);
			}

			chords.clear();
			chords.push_back(lane);
			last_tick = ev.tick;
			last_lane = lane;
		}

		return result;
	}

	float charge(const std::vector<iidx::event_t>& chart)
	{
		float result = 0;
		std::vector<iidx::event_t> charges;

		for (const auto& ev : chart)
		{
			if (ev.type == iidx::event_type_t::end_of_song)
				break;

			if (ev.type != iidx::event_type_t::note_p1 && ev.type != iidx::event_type_t::note_p2)
				continue;

			if (ev.value == 0)
				continue;

			charges.push_back(ev);
		}

		for (const auto& cn : charges)
		{
			auto end = cn.tick + cn.value;
			auto x = 0.0f;
			auto y = false, z = false;

			for (const auto& ev : chart)
			{
				if (ev.type == iidx::event_type_t::end_of_song)
					break;

				if (ev.type != iidx::event_type_t::note_p1 && ev.type != iidx::event_type_t::note_p2)
					continue;

				if (ev.tick < cn.tick || ev.tick >= end)
					continue;

				auto f = ev.tick == cn.tick ? 0.75f : 1.0f;
				if (ev.type != cn.type)
					f *= 0.5f;

				x += f;

				if (ev.tick == cn.tick)
					continue;
				if (ev.type != cn.type)
					continue;

				auto lane = ev.param % 10;
				if (lane == 7)
					continue;

				if (lane < (cn.param % 10))
					y = true;
				else if (lane > (cn.param % 10))
					z = true;
			}

			result += x * ((y && z) ? 1.1f : 1.0f);
		}

		return result;
	}

	float soflan(const std::vector<iidx::event_t>& chart)
	{
		int last_bpm = 0, last_tempo_tick = 0;
		float factor = 1.0f, result = 0.0f;

		for (const auto& ev : chart)
		{
			if (ev.type == iidx::event_type_t::tempo)
			{
				int bpm = static_cast<int>(ev.value / static_cast<float>(ev.param) + 0.5f);
				if (bpm != last_bpm)
				{
					if (last_bpm > 0)
					{
						auto a = std::min(bpm, last_bpm);
						auto b = std::max(bpm, last_bpm);

						auto diff = std::min(b / static_cast<float>(a) - 1.0f, 1.25f);
						factor = (std::log10f(diff + 0.0099999998f) / 2.2f) + 1.0f;
					}

					last_bpm = bpm;
					last_tempo_tick = ev.tick;
				}

				continue;
			}

			if (ev.type == iidx::event_type_t::note_p1 || ev.type == iidx::event_type_t::note_p2)
			{
				if (!last_tempo_tick || ev.tick > last_tempo_tick + 2000)
					continue;

				result += ((2000 - (ev.tick - last_tempo_tick)) / 100.0f) * factor;
				continue;
			}

			if (ev.type == iidx::event_type_t::end_of_song)
				break;
		}

		return result;
	}

	int map_chart(int in)
	{
		const static int map[] = {
			3, 1, 0, 2, 4, 9, 7, 6, 8, 10
		};

		return map[in];
	}

	constexpr float radar_thresholds[][4][8] =
	{
		{
			{ 10.2f, 70.00f, 1.4f, 440.00f, 160.0f, 500.00f, 1.0f, 0.0f },
			{ 15.3f, 108.0f, 2.6f, 1000.0f, 310.0f, 1010.0f, 0.5f, 1.0f },
			{ 17.9f, 122.0f, 3.9f, 2000.0f, 440.0f, 1200.0f, 0.25f, 1.5f },
			{ 20.5f, 136.0f, 6.2f, 3800.0f, 900.0f, 1450.0f, 0.25f, 1.75f },
		},
		{
			{ 10.4f, 70.00f, 1.4f, 440.00f, 145.0f, 500.00f, 1.0f, 0.0f },
			{ 15.4f, 113.0f, 2.6f, 1000.0f, 280.0f, 1000.0f, 0.5f, 1.0f },
			{ 17.7f, 125.0f, 3.9f, 2000.0f, 400.0f, 1200.0f, 0.25f, 1.5f },
			{ 20.0f, 170.0f, 7.0f, 4000.0f, 750.0f, 1500.0f, 0.25f, 1.75f },
		}
	};

	float* normalize_radar_data(float* radar, float* results, int preset)
	{
		if (preset >= 2)
			return nullptr;

		for (size_t i = 0; i < 6; i++)
		{
			auto last_threshold = 0.0f;

			for (size_t j = 0; j < 4; j++)
			{
				auto threshold = radar_thresholds[preset][j][i];
				if (j < 3 && threshold < radar[i])
				{
					last_threshold = threshold;
					continue;
				}

				results[i] = ((radar[i] - last_threshold) / (threshold - last_threshold)) * radar_thresholds[preset][j][6] + radar_thresholds[preset][j][7];
				break;
			}
		}

		return results;
	}

	void analyze_chart(const std::vector<iidx::event_t>& chart, chart_analyze_data_t& result, bool calc_radar)
	{
		if (calc_radar)
		{
			float radar_my_raw[6] = {
				notes(chart), peak(chart), scratch(chart), soflan(chart), charge(chart), chord(chart),
			};

			float radar_my[6];
			normalize_radar_data(radar_my_raw, radar_my, 0);

			result.radar.notes = radar_my[0];
			result.radar.peak = radar_my[1];
			result.radar.scratch = radar_my[2];
			result.radar.soflan = radar_my[3];
			result.radar.charge = radar_my[4];
			result.radar.chord = radar_my[5];
		}

		result.bpm.min = 100000;
		result.bpm.max = -100000;
		result.note_count = 0;

		for (const auto& ev : chart)
		{
			if (ev.type == iidx::event_type_t::end_of_song)
			{
				result.total_seconds = ev.tick / 1000.f;
				break;
			}

			if (ev.type == iidx::event_type_t::tempo)
			{
				float bpm = ev.value / static_cast<float>(ev.param);
				if (bpm < result.bpm.min)
					result.bpm.min = bpm;
				if (bpm > result.bpm.max)
					result.bpm.max = bpm;

				continue;
			}

			if (ev.type != iidx::event_type_t::note_p1 && ev.type != iidx::event_type_t::note_p2)
				continue;

			result.note_count++;
		}
	}
}
