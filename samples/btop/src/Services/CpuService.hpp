#pragma once

#include <cstdint>

#include "SystemTypes.hpp"

namespace btop
{
	class CpuService
	{
	public:
		CpuService() = delete;

		static void Collect(CpuData& out);
		static uint32_t CoreCount();
		static uint64_t LastSysTotalDelta();

		static void Prime();
	};
}
