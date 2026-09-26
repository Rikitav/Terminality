#include "MemService.hpp"

#include "../Common.hpp"

#include <psapi.h>

void btop::MemService::Collect(MemData& out)
{
	MEMORYSTATUSEX ms{ sizeof(ms) };
	if (!GlobalMemoryStatusEx(&ms))
		return;

	out.total = ms.ullTotalPhys;
	out.available = ms.ullAvailPhys;
	out.used = ms.ullTotalPhys - ms.ullAvailPhys;

	PERFORMANCE_INFORMATION pi{ sizeof(pi) };
	if (GetPerformanceInfo(&pi, sizeof(pi)))
	{
		out.cached = static_cast<uint64_t>(pi.SystemCache) * pi.PageSize;
		out.free = out.available >= out.cached ? out.available - out.cached : 0;
	}
	else
	{
		out.cached = 0;
		out.free = out.available;
	}

	// Page file figures include physical memory; approximate the swap device.
	out.swapTotal = ms.ullTotalPageFile > ms.ullTotalPhys ? ms.ullTotalPageFile - ms.ullTotalPhys : 0;
	out.swapFree = ms.ullAvailPageFile > ms.ullAvailPhys ? ms.ullAvailPageFile - ms.ullAvailPhys : 0;
	if (out.swapFree > out.swapTotal)
		out.swapFree = out.swapTotal;

	out.swapUsed = out.swapTotal - out.swapFree;
}
