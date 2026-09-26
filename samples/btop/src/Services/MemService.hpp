#pragma once

#include "SystemTypes.hpp"

namespace btop
{
	// Memory from GlobalMemoryStatusEx + GetPerformanceInfo (btop's /proc/meminfo equivalent).
	class MemService
	{
	public:
		MemService() = delete;

		static void Collect(MemData& out);
	};
}
