#pragma once

#define WEAK __declspec(selectany)

namespace game
{
	WEAK symbol<bool(const char *args)> game_preinit{0x1401AF440};
	WEAK symbol<bool()> avs_init{0x1400BA240};
}
