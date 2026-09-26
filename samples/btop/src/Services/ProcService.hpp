#pragma once

#include <vector>

#include "SystemTypes.hpp"

namespace btop
{
	// Process list from CreateToolhelp32Snapshot + per-process
	// GetProcessTimes/GetProcessMemoryInfo (btop's /proc/[0-9]* equivalent).
	class ProcService
	{
	public:
		ProcService() = delete;

		static void Collect(std::vector<ProcInfo>& out);
		static void Prime();

	private:
		static std::wstring UserFor(uint32_t pid, uint64_t createTime);
	};
}
