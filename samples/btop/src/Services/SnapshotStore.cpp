#include "SnapshotStore.hpp"

btop::SnapshotStore& btop::SnapshotStore::Current()
{
	static SnapshotStore store;
	return store;
}

void btop::SnapshotStore::Publish(std::shared_ptr<const Snapshot> snap)
{
	current_.store(std::move(snap));
	seq_.fetch_add(1, std::memory_order_release);
}

std::shared_ptr<const btop::Snapshot> btop::SnapshotStore::Load() const
{
	return current_.load();
}

uint64_t btop::SnapshotStore::Sequence() const
{
	return seq_.load(std::memory_order_acquire);
}
