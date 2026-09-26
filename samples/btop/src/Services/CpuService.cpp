#include "CpuService.hpp"

#include <algorithm>
#include <vector>

#include "../Common.hpp"

namespace
{
	using NtQuerySystemInformationFn = LONG(NTAPI*)(ULONG, PVOID, ULONG, PULONG);

	NtQuerySystemInformationFn ntQuerySystemInformation()
	{
		static NtQuerySystemInformationFn fn = reinterpret_cast<NtQuerySystemInformationFn>(
			GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQuerySystemInformation"));

		return fn;
	}

	struct ProcessorPerformanceInfo
	{
		LARGE_INTEGER idleTime;
		LARGE_INTEGER kernelTime; // includes idleTime
		LARGE_INTEGER userTime;
		LARGE_INTEGER reserved1[2];
		ULONG reserved2;
	};

	std::vector<ProcessorPerformanceInfo> queryProcessorInfo()
	{
		const auto fn = ntQuerySystemInformation();
		if (fn == nullptr)
			return {};

		// Probe with a small buffer: the API reports STATUS_INFO_LENGTH_MISMATCH
		// together with the exact required size.
		ULONG returned = 0;
		BYTE dummy[16];

		fn(8 /* SystemProcessorPerformanceInformation */, dummy, sizeof(dummy), &returned);
		if (returned == 0)
			returned = 1 << 16;

		std::vector<BYTE> buffer(returned);
		if (fn(8, buffer.data(), returned, &returned) != 0)
			return {};

		return std::vector<ProcessorPerformanceInfo>(
			reinterpret_cast<ProcessorPerformanceInfo*>(buffer.data()),
			reinterpret_cast<ProcessorPerformanceInfo*>(buffer.data() + (returned / sizeof(ProcessorPerformanceInfo)) * sizeof(ProcessorPerformanceInfo)));
	}

	double queryBaseClockMhz()
	{
		DWORD mhz = 0;
		DWORD size = sizeof(mhz);
		
		if (ERROR_SUCCESS == RegGetValueW(
			HKEY_LOCAL_MACHINE,
			L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
			L"~MHz",
			RRF_RT_REG_DWORD,
			nullptr,
			&mhz,
			&size))
		{
			return static_cast<double>(mhz);
		}

		return 0;
	}

	// Delta state, owned by the collector thread only.
	std::vector<uint64_t>& prevIdle()
	{
		static std::vector<uint64_t> value;
		return value;
	}

	std::vector<uint64_t>& prevTotal()
	{
		static std::vector<uint64_t> value;
		return value;
	}

	uint64_t& prevSysTotal()
	{
		static uint64_t value = 0;
		return value;
	}

	bool& primed()
	{
		static bool value = false;
		return value;
	}
}

void btop::CpuService::Prime()
{
	CpuData dummy;
	Collect(dummy);
}

uint32_t btop::CpuService::CoreCount()
{
	const auto infos = queryProcessorInfo();
	return static_cast<uint32_t>(infos.size());
}

void btop::CpuService::Collect(CpuData& out)
{
	const auto infos = queryProcessorInfo();
	if (infos.empty())
		return;

	const size_t count = infos.size();
	out.coreCount = static_cast<uint32_t>(count);
	out.corePercent.resize(count);
	out.freqMhz = queryBaseClockMhz();

	uint64_t idleDeltaSum = 0, totalDeltaSum = 0;

	if (prevIdle().size() != count)
	{
		prevIdle().assign(count, 0);
		prevTotal().assign(count, 0);
		primed() = false;
	}

	for (size_t i = 0; i < count; ++i)
	{
		const uint64_t idle = static_cast<uint64_t>(infos[i].idleTime.QuadPart);
		const uint64_t total = static_cast<uint64_t>(infos[i].kernelTime.QuadPart + infos[i].userTime.QuadPart);

		if (primed() && total > prevTotal()[i] && idle >= prevIdle()[i])
		{
			const uint64_t totalDelta = total - prevTotal()[i];
			const uint64_t idleDelta = idle - prevIdle()[i];
			idleDeltaSum += idleDelta;
			totalDeltaSum += totalDelta;

			out.corePercent[i] = std::clamp(static_cast<int64_t>(
				(totalDelta - idleDelta) * 100 / std::max<uint64_t>(totalDelta, 1)), 0ll, 100ll);
		}

		prevIdle()[i] = idle;
		prevTotal()[i] = total;
	}

	if (primed() && totalDeltaSum > 0)
	{
		out.totalPercent = std::clamp(static_cast<int64_t>(
			(totalDeltaSum - idleDeltaSum) * 100 / totalDeltaSum), 0ll, 100ll);
	}
	else
	{
		out.corePercent.assign(count, 0);
	}

	// Published for ProcService's cpu% normalization.
	prevSysTotal() = totalDeltaSum > 0 ? totalDeltaSum : prevSysTotal();
	primed() = true;
}

uint64_t btop::CpuService::LastSysTotalDelta()
{
	return prevSysTotal();
}
