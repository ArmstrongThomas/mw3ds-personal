#pragma once
#include <3ds.h>
#include "mwworld/player.hpp"

inline void stylusInput(PlayerInput& in, bool leftHanded, u32 held, u32 down)
{
	if (leftHanded)
	{
		in.moveX = (held & KEY_A ? 1.0f : 0.0f) - (held & KEY_Y ? 1.0f : 0.0f);
		in.moveY = (held & KEY_X ? 1.0f : 0.0f) - (held & KEY_B ? 1.0f : 0.0f);
		if (in.moveX && in.moveY)
		{
			in.moveX *= 0.70710678f;
			in.moveY *= 0.70710678f;
		}
	}
	in.attack = held & (leftHanded ? KEY_R : KEY_L);
	in.sneak = held & (leftHanded ? KEY_ZR : KEY_R);
	in.activate = !leftHanded && (down & KEY_DUP);
	in.cast = !leftHanded && (down & KEY_DRIGHT);
	in.readyToggle = !leftHanded && (down & KEY_DLEFT);
	in.jump = !leftHanded && (down & KEY_DDOWN);
	in.up = !leftHanded && (held & KEY_DDOWN);
	in.down = in.sneak;
	in.togglePov = down & KEY_SELECT;
}
