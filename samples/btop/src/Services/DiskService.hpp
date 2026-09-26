#pragma once

#include <vector>

#include "SystemTypes.hpp"

namespace btop
{
	// Volume usage from GetLogicalDriveStringsW + GetDiskFreeSpaceExW (btop's statvfs equivalent).
	class DiskService
	{
	public:
		DiskService() = delete;

		static void Collect(std::vector<DiskInfo>& out);
	};
}
