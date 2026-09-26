#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Plain data produced by the services and consumed by the widgets.
// A single immutable Snapshot is shared between the collector thread and the UI thread via std::shared_ptr,
// so large data (the process list) is never copied.
namespace btop
{
	struct ProcInfo
	{
		uint32_t pid = 0;
		std::wstring name;
		std::wstring user;
		uint32_t threads = 0;
		uint64_t mem = 0;   // working set bytes
		double cpu = 0.0;   // percent of one core
	};

	struct DiskInfo
	{
		std::wstring name;  // "C: System"
		uint64_t total = 0;
		uint64_t used = 0;
		uint64_t free = 0;
	};

	struct CpuData
	{
		std::vector<int64_t> corePercent; // 0..100 per core
		int64_t totalPercent = 0;
		double freqMhz = 0;
		uint32_t coreCount = 0;
	};

	struct MemData
	{
		uint64_t total = 0, used = 0, available = 0, cached = 0, free = 0;
		uint64_t swapTotal = 0, swapUsed = 0, swapFree = 0;
	};

	struct NetData
	{
		std::string iface;
		std::wstring ipv4;
		uint64_t downSpeed = 0, upSpeed = 0;
		uint64_t downTotal = 0, upTotal = 0;
		uint64_t downTop = 0, upTop = 0;
	};

	struct PowerData
	{
		uint64_t uptimeSec = 0;
		bool hasBattery = false;
		int batteryPercent = 0;
		bool batteryOnAc = true;
	};

	struct Snapshot
	{
		CpuData cpu;
		MemData mem;
		NetData net;
		std::vector<DiskInfo> disks;
		std::vector<ProcInfo> procs;
		PowerData power;
	};
}
