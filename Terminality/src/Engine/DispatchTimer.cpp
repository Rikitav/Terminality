
#include <cstdint>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <mutex>
#include <functional>

#include <terminality/Terminality.hpp>

using namespace terminality;

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

	std::lock_guard<std::mutex> lock(mutex_);
	tasks_.push_back(std::move(task));
}

void DispatchTimer::ProcessTasks()
{
	std::vector<std::function<void()>> currentTasks;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (tasks_.empty())
			return;

		currentTasks = std::move(tasks_);
	}

	for (const auto& task : currentTasks)
	{
		try
		{
			task();
		}
		catch (const std::exception& e)
		{
			// ...
		}
	}
}

bool DispatchTimer::IsRunning() const
{
	return running_.load();
}

bool DispatchTimer::IsResizing() const
{
	return isResizing_;
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
}

void DispatchTimer::BeginResize()
{
	isResizing_ = true;
	resizeDebounceTimer_ = RESIZE_DELAY;
}

std::chrono::milliseconds DispatchTimer::GetRemainingFrameTime(int targetFPS)
{
	const auto targetFrameTime = std::chrono::milliseconds(1000 / targetFPS);
	auto workTime = std::chrono::high_resolution_clock::now() - frameStart_;
	auto waitTime = targetFrameTime - std::chrono::duration_cast<std::chrono::milliseconds>(workTime);

	if (waitTime.count() < 0)
		return std::chrono::milliseconds(0);

	return waitTime;
}