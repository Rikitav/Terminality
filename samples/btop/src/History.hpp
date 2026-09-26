#pragma once

#include <cstddef>
#include <deque>
#include <vector>

// Shared rolling histories, fed once per collection tick and read by the graph controls.
// Mirrors the deque trimming btop does in btop.cpp.
namespace btop
{
	struct History
	{
		static constexpr size_t MaxSamples = 1024; // >= 2x the widest plausible graph

		std::deque<int64_t> cpuTotal;              // 0..100
		std::vector<std::deque<int64_t>> cores;    // 0..100 per core, trimmed to 40 like btop
		std::deque<int64_t> netDown;               // bytes/s
		std::deque<int64_t> netUp;                 // bytes/s

		// MEM box rows: percent history per stat.
		std::deque<int64_t> memUsed;
		std::deque<int64_t> memAvailable;
		std::deque<int64_t> memCached;
		std::deque<int64_t> memFree;
		std::deque<int64_t> swapUsed;
		std::deque<int64_t> swapFree;

		void ensureCores(size_t count)
		{
			if (cores.size() == count)
				return;

			cores.clear();
			cores.resize(count);
		}

		static void push(std::deque<int64_t>& deque, int64_t value, size_t max = MaxSamples)
		{
			deque.push_back(value);
			while (deque.size() > max)
				deque.pop_front();
		}
	};
}
