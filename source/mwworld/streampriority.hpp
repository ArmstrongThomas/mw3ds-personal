#pragma once
#include <cmath>

inline float streamCellDistance(int gx, int gy, float x, float y)
{
	float dx = fmaxf(fmaxf(gx * 8192.0f - x, x - (gx + 1) * 8192.0f), 0.0f);
	float dy = fmaxf(fmaxf(gy * 8192.0f - y, y - (gy + 1) * 8192.0f), 0.0f);
	return dx * dx + dy * dy;
}
