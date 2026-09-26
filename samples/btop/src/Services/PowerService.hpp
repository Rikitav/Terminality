#pragma once

#include "SystemTypes.hpp"

namespace btop
{
	// Uptime and battery from GetTickCount64 / GetSystemPowerStatus.
	class PowerService
	{
	public:
		PowerService() = delete;

		static void Collect(PowerData& out);
	};
}
