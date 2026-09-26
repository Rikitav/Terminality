#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

#include "SnapshotStore.hpp"

namespace btop
{
	// Runs all services on a dedicated thread so the UI dispatcher never
	// blocks on data collection. Fresh snapshots go to the SnapshotStore and
	// are handed to the UI thread through a dispatch callback (which
	// marshals onto the UI loop via DispatchTimer::InvokeAsync).
	class Collector
	{
	public:
		using DispatchFn = std::function<void(std::shared_ptr<const Snapshot>)>;

	private:
		std::thread thread_;
		std::mutex mutex_;
		std::condition_variable cv_;
		bool stop_ = false;
		bool immediate_ = false;
		int intervalMs_ = 2000;
		DispatchFn dispatch_;

		void Run();
		std::shared_ptr<const Snapshot> CollectAll();
	
	public:
		Collector() = default;
		~Collector();

		void Start(int intervalMs, DispatchFn dispatch);
		void Stop();
		void SetInterval(int ms);
		void RequestImmediate();
	};
}
