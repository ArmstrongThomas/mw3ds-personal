#include "platform/battery.hpp"
#include <3ds.h>

namespace
{
int s_percent = -1;
u64 s_nextPoll = 0;
bool s_active = false;
}

void batteryInit()
{
#ifdef __3DS__
	s_active = R_SUCCEEDED(mcuHwcInit());
#endif
	s_nextPoll = 0;
	batteryUpdate();
}

void batteryUpdate()
{
#ifdef __3DS__
	u64 now = osGetTime();
	if (!s_active || now < s_nextPoll)
		return;
	s_nextPoll = now + 10000;
	u8 percent = 0;
	s_percent = R_SUCCEEDED(MCUHWC_GetBatteryLevel(&percent)) && percent <= 100 ? percent : -1;
#endif
}

void batteryExit()
{
#ifdef __3DS__
	if (s_active)
		mcuHwcExit();
#endif
	s_active = false;
	s_percent = -1;
}

int batteryPercent()
{
	return s_percent;
}
