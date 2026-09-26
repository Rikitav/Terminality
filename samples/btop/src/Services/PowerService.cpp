#include "PowerService.hpp"

#include "../Common.hpp"

void btop::PowerService::Collect(PowerData& out)
{
	out.uptimeSec = GetTickCount64() / 1000;

	SYSTEM_POWER_STATUS power{};
	if (GetSystemPowerStatus(&power) && power.BatteryLifePercent != 255)
	{
		out.hasBattery = true;
		out.batteryPercent = power.BatteryLifePercent;
		out.batteryOnAc = power.ACLineStatus == 1;
	}
	else
	{
		out.hasBattery = false;
	}
}
