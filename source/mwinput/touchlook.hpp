#pragma once

struct TouchLook
{
	static constexpr int left = 6, top = 48, width = 308, height = 126;
	static constexpr int actionLeft = 6, actionTop = 179, actionStep = 36, actionWidth = 33, actionHeight = 26;
	static int actionAt(int x, int y)
	{
		if (x < actionLeft || x >= actionLeft + 5 * actionStep || y < actionTop || y >= actionTop + actionHeight)
			return -1;
		return (x - actionLeft) % actionStep < actionWidth ? (x - actionLeft) / actionStep : -1;
	}
	enum Owner { NONE, UI, LOOK, BLOCKED } owner = NONE;
	int lastX = 0, lastY = 0, dx = 0, dy = 0;

	static bool contains(int x, int y)
	{
		return x >= left && x < left + width && y >= top && y < top + height;
	}

	// Ownership lasts through release, including after a menu opens or the stylus leaves the pad.
	bool update(bool touching, bool eligible, int x, int y)
	{
		dx = dy = 0;
		if (!touching)
		{
			bool consumed = owner == LOOK || owner == BLOCKED;
			owner = NONE;
			return consumed;
		}
		if (owner == NONE)
			owner = eligible && contains(x, y) ? LOOK : UI;
		else if (owner == LOOK)
		{
			if (!eligible)
				owner = BLOCKED;
			else
			{
				dx = x - lastX;
				dy = y - lastY;
			}
		}
		else if (owner == UI && eligible && contains(x, y))
			owner = BLOCKED;
		lastX = x;
		lastY = y;
		return owner == LOOK || owner == BLOCKED;
	}
};
