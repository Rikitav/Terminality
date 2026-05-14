module terminality;

import std;
import std.compat;

using namespace terminality;

DispatchTimer& DispatchTimer::Current()
{
	static DispatchTimer dispatcher;
	return dispatcher;
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