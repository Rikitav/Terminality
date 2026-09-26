#include "Collector.hpp"

#include <chrono>

#include "CpuService.hpp"
#include "DiskService.hpp"
#include "MemService.hpp"
#include "NetService.hpp"
#include "PowerService.hpp"
#include "ProcService.hpp"

btop::Collector::~Collector()
{
	Stop();
}

void btop::Collector::Start(int intervalMs, DispatchFn dispatch)
{
	std::lock_guard lock(mutex_);
	intervalMs_ = intervalMs;
	dispatch_ = std::move(dispatch);
	stop_ = false;

	// Prime the delta-based services so the first snapshot has real data.
	CpuService::Prime();
	NetService::Prime();
	ProcService::Prime();

	// First snapshot synchronously so the UI has data on the first frame.
	std::shared_ptr<const Snapshot> first = CollectAll();
	SnapshotStore::Current().Publish(first);

	if (dispatch_)
		dispatch_(first);

	thread_ = std::thread(&Collector::Run, this);
}

void btop::Collector::Stop()
{
	{
		std::lock_guard lock(mutex_);
		stop_ = true;
	}
	cv_.notify_all();
	if (thread_.joinable())
		thread_.join();
}

void btop::Collector::SetInterval(int ms)
{
	{
		std::lock_guard lock(mutex_);
		intervalMs_ = ms;
	}

	cv_.notify_all();
}

void btop::Collector::RequestImmediate()
{
	{
		std::lock_guard lock(mutex_);
		immediate_ = true;
	}
	
	cv_.notify_all();
}

void btop::Collector::Run()
{
	for (;;)
	{
		std::unique_lock lock(mutex_);
		cv_.wait_for(lock, std::chrono::milliseconds(intervalMs_), [this]()
		{
				return stop_ || immediate_;
		});
		
		if (stop_)
			return;
	
		immediate_ = false;

		std::shared_ptr<const Snapshot> snap = CollectAll();
		SnapshotStore::Current().Publish(snap);

		if (dispatch_)
			dispatch_(snap);
	}
}

std::shared_ptr<const btop::Snapshot> btop::Collector::CollectAll()
{
	auto snap = std::make_shared<Snapshot>();

	// CPU first: ProcService normalizes process cpu% by its delta.
	CpuService::Collect(snap->cpu);
	MemService::Collect(snap->mem);
	NetService::Collect(snap->net);
	DiskService::Collect(snap->disks);
	ProcService::Collect(snap->procs);
	PowerService::Collect(snap->power);

	return snap;
}
