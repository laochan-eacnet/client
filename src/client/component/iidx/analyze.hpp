#pragma once

namespace iidx::analyze
{
	struct chart_analyze_data_t
	{
		size_t note_count;
		float total_seconds;

		struct
		{
			float min;
			float max;
		} bpm;

		struct
		{
			float notes;
			float peak;
			float scratch;
			float soflan;
			float charge;
			float chord;
		} radar;
	};

	int map_chart(int in);
	void analyze_chart(const std::vector<iidx::event_t>& chart, chart_analyze_data_t& result, bool calc_radar);
}
