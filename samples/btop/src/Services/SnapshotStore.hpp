#pragma once

#include <atomic>
#include <memory>

#include "SystemTypes.hpp"

namespace btop
{
	// Copy-on-write snapshot storage:
	// * the collector thread publishes an immutable Snapshot,
	// * readers (UI thread) get the same shared_ptr,
	// * so large data (the process list) is never copied.
	class SnapshotStore
	{
	public:
		static SnapshotStore& Current();

		void Publish(std::shared_ptr<const Snapshot> snap);
		std::shared_ptr<const Snapshot> Load() const;
		uint64_t Sequence() const;

	private:
		std::atomic<std::shared_ptr<const Snapshot>> current_;
		std::atomic<uint64_t> seq_{ 0 };
	};
}
