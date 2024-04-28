#pragma once

namespace chart_modifier
{
	enum modifier_t : uint32_t
	{
		none = 0,
		all_scratch = 1 << 0,
		all_charge = 1 << 1,
	};

	inline modifier_t operator|(modifier_t lhs, modifier_t rhs)
	{
		return static_cast<modifier_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
	}

	inline uint32_t operator&(modifier_t lhs, modifier_t rhs)
	{
		return static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs);
	}

	void set(modifier_t modifier);
	modifier_t get();
}
