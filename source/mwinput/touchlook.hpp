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
	enum Owner { NONE, UI, LOOK, MAP, BLOCKED } owner = NONE;
	int lastX = 0, lastY = 0, dx = 0, dy = 0;

	static int mapLeft(bool leftHanded) { return leftHanded ? 210 : 14; }
	static bool mapPicture(int x, int y, bool leftHanded)
	{
		int mx = mapLeft(leftHanded);
		return x >= mx && x < mx + 96 && y >= 68 && y < 164 && !(x >= mx + 48 && y >= 144);
	}
	static bool contains(int x, int y, bool leftHanded = false, int effects = 0)
	{
		int mx = mapLeft(leftHanded), ex = leftHanded ? 14 : 118;
		bool mapButton = x >= mx && x < mx + 96 && y >= 48 && y < 164;
		bool effectButton = x >= ex && x < ex + effects * 20 && y >= 148 && y < 172;
		return !mapButton && !effectButton && x >= left && x < left + width && y >= top && y < top + height;
	}

	// Ownership lasts through release, including after a menu opens or the stylus leaves the pad.
	bool update(bool touching, bool eligible, int x, int y, bool leftHanded = false, int effects = 0)
	{
		dx = dy = 0;
		if (!touching)
		{
			bool consumed = owner == LOOK || owner == BLOCKED;
			owner = NONE;
			return consumed;
		}
		if (owner == NONE)
			owner = eligible && contains(x, y, leftHanded, effects) ? LOOK
				: eligible && mapPicture(x, y, leftHanded) ? MAP : UI;
		else if (owner == MAP && !eligible)
			owner = BLOCKED;
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
		else if (owner == UI && eligible && contains(x, y, leftHanded, effects))
			owner = BLOCKED;
		lastX = x;
		lastY = y;
		return owner == LOOK || owner == BLOCKED;
	}
};
