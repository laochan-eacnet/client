#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/memory.hpp>
#include <game/game.hpp>

#include "chart_modifier.hpp"

namespace chart_modifier
{
	modifier_t modifier_flag = modifier_t::none;

	void set(modifier_t modifier)
	{
		modifier_flag = modifier;
	}

	modifier_t get()
	{
		return modifier_flag;
	}

	constexpr int MAX_EVENT = 0x19800 / 8;

	std::vector<game::event_t>::iterator find_sample(std::vector<game::event_t>& chart, game::event_type_t type, int column, std::vector<game::event_t>::iterator start_from)
	{
		if (!chart.size())
			return chart.end();

		for (auto j = start_from - 1; j > chart.begin(); j--)
		{
			if ((type + 2) == j->type && j->param == column)
			{
				return j;
			}
		}

		return chart.end();
	}

	void all_scratch_modifier(std::vector<game::event_t>& chart)
	{
		std::vector<game::event_t> events_to_add;

		for (auto i = chart.begin(); i < chart.end(); i++)
		{
			if (i->type != game::note_p1 && i->type != game::note_p2)
				continue;

			// don't move cn
			if (i->value)
				continue;

			for (auto j = i + 1; j < chart.end(); j++)
			{
				if (j->type != i->type || j->tick < i->tick /* but how? */)
					continue;

				if (j->tick > i->tick)
					break;

				// ok we found there's already scratch...
				if (j->param == 7)
				{
					i = j;
					break;
				}

				if ((j->type == game::note_p1 && i->param < j->param) ||
					(j->type == game::note_p2 && i->param > j->param)
					) {
					i = j;
					continue;
				}
			}

			// just skips if there's already scratch...
			if (i->param == 7)
				continue;

			auto sample = find_sample(chart, i->type, i->param, i);
			auto old_sample = find_sample(chart, i->type, 7, i);
			auto created_sample_before = find_sample(events_to_add, i->type, 7, events_to_add.end());

			assert(sample != chart.end());

			auto sample_to_compare = old_sample == chart.end() ?
				(created_sample_before == events_to_add.end() ? nullptr : created_sample_before._Ptr) :
				(created_sample_before == events_to_add.end() ? old_sample._Ptr :
					(old_sample->tick > created_sample_before->tick ? old_sample._Ptr : created_sample_before._Ptr));

			if (!sample_to_compare || sample_to_compare->value != sample->value)
			{
				events_to_add.push_back
				(
					game::event_t
					{
						.tick = i->tick - 1,
						.type = static_cast<game::event_type_t>(i->type + 2),
						.param = i->param,
						.value = sample->value,
					}
				);

				if (sample_to_compare)
				{
					events_to_add.push_back
					(
						game::event_t
						{
							.tick = i->tick + 1,
							.type = static_cast<game::event_type_t>(i->type + 2),
							.param = i->param,
							.value = sample_to_compare->value,
						}
					);
				}
			}

			i->param = 7;
		}

		chart.insert(chart.end(), events_to_add.begin(), events_to_add.end());
		std::stable_sort(chart.begin(), chart.end(), [](const game::event_t& a, const game::event_t& b)
			{
				return a.tick < b.tick;
			});
	}

	void all_charge_note_modifier(std::vector<game::event_t>& chart)
	{
		for (auto i = chart.begin(); i < chart.end(); i++)
		{
			if (i->type != game::note_p1 && i->type != game::note_p2)
				continue;

			// don't make scratches into bss
			if (i->param == 7)
				continue;

			// already cn
			if (i->value)
				continue;

			auto next = std::find_if(i + 1, chart.end(), [i](const game::event_t& e)
				{
					return e.type == i->type && e.param == i->param && e.tick > i->tick;
				});

			if (next == chart.end())
				continue;

			auto sample = find_sample(chart, next->type, next->param, next);

			assert(sample != chart.end());

			i->value = static_cast<int16_t>(next->tick - i->tick);

			next->type = game::bgm;
			next->param = 8;
			next->value = sample->value;
		}
	}

	utils::hook::detour load_chart_hook;
	bool load_chart(game::event_t* raw_chart, int unk, const char* chart_path, int difficulty)
	{
		auto success = load_chart_hook.invoke<bool>(raw_chart, unk, chart_path, difficulty);

		if (!success)
			return false;

		printf("loading chart %s, diff: %d\n", chart_path, difficulty);

		auto* eos = std::find_if(raw_chart, raw_chart + MAX_EVENT, [](const game::event_t& e) {
			return e.type == game::end_of_song;
			});

		if (modifier_flag == modifier_t::none)
			return true;

		std::vector<game::event_t> chart;
		chart.assign(raw_chart, eos + 1);

		if (modifier_flag & modifier_t::all_scratch)
			all_scratch_modifier(chart);

		if (modifier_flag & modifier_t::all_charge)
			all_charge_note_modifier(chart);

		std::memcpy(raw_chart, chart.data(), chart.size() * sizeof(game::event_t));

		return true;
	}

	utils::hook::detour get_option_str_hook;
	void get_option_str(DWORD* a1, game::string* option_str, int a3, __int64 a4)
	{
		get_option_str_hook.invoke<void>(a1, option_str, a3, a4);

		if (option_str->data() == "OFF/OFF"s)
			return option_str->clear();

		if (modifier_flag & modifier_t::all_scratch)
		{
			if (option_str->size() > 0)
				option_str->append(", ");

			option_str->append("ALL SCRATCH");
		}

		if (modifier_flag & modifier_t::all_charge)
		{
			if (option_str->size() > 0)
				option_str->append(", ");

			option_str->append("ALL CHARGE");
		}
	}

#ifdef DEBUG
	bool test_fps(float* a1, int a2)
	{
		return true;
	}
#endif

	bool report_result_export_request_property(game::eacnet_request_post_s *_this, void* a1, void* a2)
	{
		game::property_node_create(_this->eacnet_property.property, nullptr, game::NODE_TYPE_u32, "/p2d/params/modifier", static_cast<uint32_t>(modifier_flag));
		return game::EacnetRequestPost::OnRequestPropertyExported(_this, a1, a2);
	}

	class component final : public component_interface
	{
	public:
		void post_load() override
		{
#ifdef DEBUG
			utils::hook::jump(0x14016DC30, test_fps);
#endif

			load_chart_hook.create(0x14011B720, load_chart);
			get_option_str_hook.create(0x140131820, get_option_str);

			utils::hook::set(0x140423778, report_result_export_request_property);
		}
	};
}

REGISTER_COMPONENT(chart_modifier::component)
