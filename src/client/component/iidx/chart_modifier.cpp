#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/memory.hpp>
#include <game/game.hpp>

#include "chart_modifier.hpp"

namespace chart_modifier
{
	modifier_t modifier_flag = modifier_t::none;
	int8_t fran_template[7] = { 0, 1, 2, 3, 4, 5, 6 };
	int8_t fran_template_inv[7] = { 0, 1, 2, 3, 4, 5, 6 };

	void set(modifier_t modifier)
	{
		modifier_flag = modifier;
	}

	void set_fran(int8_t *new_template)
	{
		memcpy(fran_template, new_template, 7);

		for (int8_t i = 0; i < 7; i++)
			fran_template_inv[fran_template[i]] = i;
	}

	modifier_t get()
	{
		return modifier_flag;
	}

	constexpr int MAX_EVENT = 0x19800 / 8;
	using chart_ro_t = std::span<game::event_t>;
	using chart_t = std::vector<game::event_t>;

	auto find_event_backward(const chart_ro_t& chart, game::event_type_t type, int8_t param, chart_ro_t::iterator start_from)
	{
		if (!chart.size())
			return chart.end();

		for (auto j = start_from - 1; j >= chart.begin(); j--)
		{
			if (type == j->type && j->param == param)
			{
				return j;
			}
		}

		return chart.end();
	}

	auto find_event_forward(const chart_ro_t& chart, game::event_type_t type, int8_t param, chart_ro_t::iterator start_from)
	{
		if (!chart.size())
			return chart.end();

		for (auto j = start_from + 1; j < chart.end(); j++)
		{
			if (type == j->type && j->param == param)
			{
				return j;
			}
		}

		return chart.end();
	}

	chart_t d4dj_modifier(const chart_ro_t& chart)
	{
		chart_t output;
		game::event_t bpm_current = {};
		int last_scratch_back = 0;

		for (auto& ev : chart)
		{
			if (ev.type == game::tempo)
				bpm_current = ev;

			if (ev.type < 2 && ev.param == 7)
			{
				auto bpm = bpm_current.value / static_cast<float>(bpm_current.param);
				auto sb_time = 7500 / bpm;

				if (ev.tick > last_scratch_back)
				{
					last_scratch_back = static_cast<int>(ev.tick + sb_time);

					output.push_back(game::event_t
						{
						.tick = ev.tick,
						.type = game::tempo,
						.param = bpm_current.param,
						.value = static_cast<int16_t>(-bpm_current.value),
						});

					output.push_back(game::event_t
						{
						.tick = last_scratch_back,
						.type = game::tempo,
						.param = bpm_current.param,
						.value = bpm_current.value,
						});
				}
			}

			output.push_back(ev);
		}

		std::stable_sort(output.begin(), output.end(), [](const game::event_t& a, const game::event_t& b)
			{
				return a.tick < b.tick;
			});

		return output;
	}

	chart_t all_scratch_modifier_2dxplus(const chart_ro_t& chart)
	{
		chart_t output;
		std::array<game::event_t, 8> current_samples_p1{};
		std::array<game::event_t, 8> current_samples_p2{};

		int16_t scratch_sample = 0;
		int scratch_tick = 0;

		for (auto it = chart.begin(); it < chart.end(); it++)
		{
			auto& ev = *it;

			if (ev.type == game::sample_p1)
			{
				current_samples_p1[ev.param] = ev;
			}

			if (ev.type == game::sample_p2)
			{
				current_samples_p2[ev.param] = ev;
			}

			else if (ev.type < 2 && ev.param == 7)
			{
				auto& sample = (ev.type == game::note_p1 ? current_samples_p1 : current_samples_p2)[ev.param];
				if (scratch_sample != sample.value)
				{
					output.push_back(game::event_t
						{
							.tick = scratch_tick + (ev.tick - scratch_tick) / 2,
							.type = static_cast<game::event_type_t>(ev.type + 2),
							.param = 7,
							.value = sample.value,
						});
				}

				scratch_sample = sample.value;
				scratch_tick = ev.tick + ev.value;
			}

			else if (ev.type < 2 && !ev.value && ev.tick - scratch_tick >= 50)
			{
				auto next_scr = find_event_forward(chart, ev.type, 7, it);

				if (next_scr == chart.end() || next_scr->tick - ev.tick >= 50)
				{
					auto& sample = (ev.type == game::note_p1 ? current_samples_p1 : current_samples_p2)[ev.param];

					if (scratch_sample != sample.value)
					{
						output.push_back(game::event_t
							{
								.tick = scratch_tick + (ev.tick - scratch_tick) / 2,
								.type = static_cast<game::event_type_t>(ev.type + 2),
								.param = 7,
								.value = sample.value,
							});
					}

					ev.param = 7;
					scratch_sample = sample.value;
					scratch_tick = ev.tick + ev.value;

					printf("moved %d - %d to scratch.\n", ev.tick, ev.param);
				}
			}

			output.push_back(ev);
		}

		return output;
	}

	chart_t all_charge_note_modifier(chart_ro_t& chart)
	{
		chart_t output;

		for (auto i = chart.begin(); i < chart.end(); i++)
		{
			if (i->type < 2 && i->param != 7 && !i->value)
			{
				auto next = std::find_if(i + 1, chart.end(), [i](const game::event_t& e)
					{
						return e.type == i->type && e.param == i->param && e.tick > i->tick;
					});

				if (next != chart.end() && !next->value)
				{
					auto sample = find_event_backward(chart, static_cast<game::event_type_t>(next->type + 2), next->param, next);
					assert(sample != chart.end());

					i->value = static_cast<int16_t>(next->tick - i->tick);
					next->type = game::bgm;
					next->param = 0;
					next->value = sample->value;
				}
			}

			output.push_back(*i);
		}

		return output;
	}

	chart_t regular_speed_modifier(chart_ro_t& chart)
	{
		chart_t output;

		for (auto& ev : chart)
		{
			if (ev.type == game::tempo)
			{
				ev.param = 1;
				ev.value = 150;
			}

			output.push_back(ev);
		}

		return output;
	}

	chart_t fixed_random_modifier(chart_ro_t& chart)
	{
		chart_t output;

		for (auto& ev : chart)
		{
			if (ev.type < game::tempo && ev.param != 7)
				ev.param = fran_template_inv[ev.param];

			output.push_back(ev);
		}

		return output;
	}

	utils::hook::detour post_load_chart_hook;
	void post_load_chart(game::event_t* raw_chart, int unk)
	{
		if (modifier_flag == modifier_t::none)
			return post_load_chart_hook.invoke<void>(raw_chart, unk);

		auto* eos = std::find_if(raw_chart, raw_chart + MAX_EVENT, [](const game::event_t& e) {
			return e.type == game::end_of_song;
			});

		chart_t modded_chart;
		auto chart = chart_ro_t{ raw_chart, eos + 1 };

		if (modifier_flag & modifier_t::all_scratch)
		{
			modded_chart = all_scratch_modifier_2dxplus(chart);
			chart = chart_ro_t{ modded_chart.begin(), modded_chart.end() };
		}

		if (modifier_flag & modifier_t::all_charge)
		{
			modded_chart = all_charge_note_modifier(chart);
			chart = chart_ro_t{ modded_chart.begin(), modded_chart.end() };
		}

		if (modifier_flag & modifier_t::regular_speed)
		{
			modded_chart = regular_speed_modifier(chart);
			chart = chart_ro_t{ modded_chart.begin(), modded_chart.end() };
		}

		if (modifier_flag & modifier_t::d4dj)
		{
			modded_chart = d4dj_modifier(chart);
			chart = chart_ro_t{ modded_chart.begin(), modded_chart.end() };
		}

		if (modifier_flag & modifier_t::fixed_random)
		{
			modded_chart = fixed_random_modifier(chart);
			chart = chart_ro_t{ modded_chart.begin(), modded_chart.end() };
		}

		std::memcpy(raw_chart, chart.data(), chart.size() * sizeof(game::event_t));

		return post_load_chart_hook.invoke<void>(raw_chart, unk);
	}

	utils::hook::detour get_option_str_hook;
	void get_option_str(DWORD* a1, game::string* option_str, int a3, __int64 a4)
	{
		get_option_str_hook.invoke<void>(a1, option_str, a3, a4);

		if (modifier_flag == modifier_t::none)
			return;

		if (option_str->data() == "OFF"s)
			option_str->clear();

		if (modifier_flag & modifier_t::all_scratch)
		{
			if (option_str->size() > 0)
				option_str->append(", ");

			option_str->append("ALL-SCR");
		}

		if (modifier_flag & modifier_t::all_charge)
		{
			if (option_str->size() > 0)
				option_str->append(", ");

			option_str->append("ALL-CN");
		}

		if (modifier_flag & modifier_t::d4dj)
		{
			if (option_str->size() > 0)
				option_str->append(", ");

			option_str->append("D4DJ");
		}

		if (modifier_flag & modifier_t::regular_speed)
		{
			if (option_str->size() > 0)
				option_str->append(", ");

			option_str->append("REGUL-SPD");
		}

		if (modifier_flag & modifier_t::fixed_random)
		{
			if (option_str->size() > 0)
				option_str->append(", ");

			option_str->append("F-RAN (");
			
			for (size_t i = 0; i < 7; i++)
			{
				option_str->append(std::to_string(fran_template[i] + 1));
			}

			option_str->append("}");
		}
	}

#ifdef DEBUG
	bool test_fps(float* a1, int a2)
	{
		return true;
	}
#endif

	bool report_result_export_request_property(game::eacnet_request_post_s* _this, void* a1, void* a2)
	{
		game::property_node_create(_this->eacnet_property.property, nullptr, game::NODE_TYPE_u32, "/p2d/params/modifier", static_cast<uint32_t>(modifier_flag & ~modifier_t::fixed_random));
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

			post_load_chart_hook.create(0x14011B940, post_load_chart);
			get_option_str_hook.create(0x140131820, get_option_str);

			utils::hook::set(0x140423778, report_result_export_request_property);

			// allow negtive bpm (movzx -> movsx)
			utils::hook::set<uint8_t>(0x14011C4C7, 0xBF);
	}
};
}

REGISTER_COMPONENT(chart_modifier::component, launcher::game::iidx)
