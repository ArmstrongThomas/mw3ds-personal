#pragma once

void batteryInit();
void batteryUpdate();
void batteryExit();
int batteryPercent(); // -1 when the service or reading is unavailable
