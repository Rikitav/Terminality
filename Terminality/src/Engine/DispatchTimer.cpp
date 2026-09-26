
#include <cstdint>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <functional>

#include <terminality/Engine/DispatchTimer.hpp>
#include <terminality/Framework/HostApplication.hpp>

using namespace terminality;

using DueTimersMap = std::vector<std::pair<std::uint64_t, std::function<void()>>>;

DispatchTimer& DispatchTimer::Current()
{
	static DispatchTimer dispatcher;
	return dispatcher;
}

void DispatchTimer::SetUIThread()
{
	uiThreadId_ = std::this_thread::get_id();
}

bool DispatchTimer::CheckAccess() const
{
	return std::this_thread::get_id() == uiThreadId_;
}

void DispatchTimer::VerifyAccess() const
{
	if (!CheckAccess())
	{
		throw std::logic_error("Invalid cross-thread operation. You must use DispatchTimer::InvokeAsync to access this object.");
	}
}

void DispatchTimer::InvokeAsync(std::function<void()> task)
{
	if (!task)
		return;

	{
		std::lock_guard<std::mutex> lock(mutex_);
		tasks_.push_back(std::move(task));
	}

	HostBackend::SignalInput();
}

void DispatchTimer::ProcessTasks()
{
	std::vector<std::function<void()>> currentTasks;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (!tasks_.empty())
			currentTasks = std::move(tasks_);
	}

	for (const auto& task : currentTasks)
	{
		try
		{
			task();
		}
		catch (const std::exception&)
		{
			// ...
		}
		catch (...)
		{
			// ...
		}
	}

	std::lock_guard<std::mutex> lock(mutex_);
	if (tasks_.empty())
		HostBackend::ResetInputSignal();
}

DispatchTimer::TimerHandle& DispatchTimer::TimerHandle::operator=(TimerHandle&& other) noexcept
{
	if (this != &other)
	{
		Cancel();
		id_ = std::exchange(other.id_, 0);
	}

	return *this;
}

DispatchTimer::TimerHandle::~TimerHandle()
{
	Cancel();
}

void DispatchTimer::TimerHandle::Cancel()
{
	if (id_ == 0)
		return;

	DispatchTimer::Current().CancelTimer(std::exchange(id_, 0));
}

bool DispatchTimer::TimerHandle::IsActive() const
{
	if (id_ == 0)
		return false;

	DispatchTimer& timer = DispatchTimer::Current();
	std::lock_guard<std::mutex> lock(timer.mutex_);
	return timer.timers_.find(id_) != timer.timers_.end();
}

void DispatchTimer::CancelTimer(std::uint64_t id)
{
	std::lock_guard<std::mutex> lock(mutex_);
	timers_.erase(id);
}

DispatchTimer::TimerHandle DispatchTimer::SetInterval(std::chrono::milliseconds interval, std::function<void()> callback)
{
	if (!callback)
		return TimerHandle();

	TimerEntry entry;
	entry.callback = std::move(callback);
	entry.period = interval;
	entry.repeat = true;

	std::lock_guard<std::mutex> lock(mutex_);
	entry.nextFire = totalTime_ + interval.count() / 1000.0f;

	const std::uint64_t id = nextTimerId_++;
	timers_.emplace(id, std::move(entry));

	// Wake the UI thread: a short delay may already be due.
	HostBackend::SignalInput();
	return TimerHandle(id);
}

DispatchTimer::TimerHandle DispatchTimer::SetTimeout(std::chrono::milliseconds delay, std::function<void()> callback)
{
	if (!callback)
		return TimerHandle();

	TimerEntry entry;
	entry.callback = std::move(callback);
	entry.period = delay;
	entry.repeat = false;

	std::lock_guard<std::mutex> lock(mutex_);
	entry.nextFire = totalTime_ + delay.count() / 1000.0f;

	const std::uint64_t id = nextTimerId_++;
	timers_.emplace(id, std::move(entry));

	// Wake the UI thread: a short delay may already be due.
	HostBackend::SignalInput();
	return TimerHandle(id);
}

bool DispatchTimer::IsRunning() const
{
	return running_.load();
}

bool DispatchTimer::IsResizing() const
{
	return isResizing_;
}

float DispatchTimer::DeltaTime() const
{
	return deltaTime_;
}

float DispatchTimer::TotalTime() const
{
	return totalTime_;
}

void DispatchTimer::Start()
{
	running_.store(true);
	lastTime_ = std::chrono::high_resolution_clock::now();
}

void DispatchTimer::Stop()
{
	running_.store(false);
}

void DispatchTimer::Tick()
{
	if (!running_.load())
		return;

	frameStart_ = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> dt = frameStart_ - lastTime_;
	lastTime_ = frameStart_;

	deltaTime_ = dt.count();
	totalTime_ += deltaTime_;

	TickEvent.Emit(deltaTime_);

	if (isResizing_)
	{
		resizeDebounceTimer_ -= deltaTime_;
		if (resizeDebounceTimer_ <= 0.0f)
		{
			isResizing_ = false;
			ResizeFinishedEvent.Emit();
		}
	}

	DueTimersMap dueTimers;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		for (const auto& [id, timer] : timers_)
		{
			if (totalTime_ >= timer.nextFire)
				dueTimers.emplace_back(id, timer.callback);
		}
	}

	for (const auto& [id, callback] : dueTimers)
	{
		if (callback)
			callback();

		std::lock_guard<std::mutex> lock(mutex_);
		const auto it = timers_.find(id);
		if (it == timers_.end())
			continue; // cancelled (handle destroyed or Cancel()) while firing

		if (it->second.repeat)
			it->second.nextFire = totalTime_ + it->second.period.count() / 1000.0f;
		else
			timers_.erase(it);
	}
}

void DispatchTimer::BeginResize()
{
	isResizing_ = true;
	resizeDebounceTimer_ = RESIZE_DELAY;
}

std::chrono::milliseconds DispatchTimer::GetRemainingFrameTime(int targetFPS)
{
	if (targetFPS <= 0)
		return std::chrono::milliseconds(0);

	const auto targetFrameTime = std::chrono::milliseconds(1000 / targetFPS);
	auto workTime = std::chrono::high_resolution_clock::now() - frameStart_;
	auto waitTime = targetFrameTime - std::chrono::duration_cast<std::chrono::milliseconds>(workTime);

	if (waitTime.count() < 0)
		return std::chrono::milliseconds(0);

	return waitTime;
}