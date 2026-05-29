module;

#include <cstdint>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <functional>

export module terminality:DispatchTimer;

import :Event;

export namespace terminality
{
	class DispatchTimer
	{
		std::optional<std::thread::id> uiThreadId_;
		std::mutex mutex_;
		std::vector<std::function<void()>> tasks_;

		float deltaTime_ = 0.0f;
		float totalTime_ = 0.0f;
		std::atomic<bool> running_ = false;

		// ticking
		std::chrono::time_point<std::chrono::high_resolution_clock> lastTime_;
		std::chrono::time_point<std::chrono::high_resolution_clock> frameStart_;

		// debouncing
		bool isResizing_ = false;
		float resizeDebounceTimer_ = 0.0f;
		const float RESIZE_DELAY = 0.1f;

		DispatchTimer() = default;
		DispatchTimer(const DispatchTimer&) = delete;
		DispatchTimer& operator=(const DispatchTimer&) = delete;

	public:
		Event<float> TickEvent;
		Event<> ResizeFinishedEvent;

		static DispatchTimer& Current();

		void SetUIThread();
		bool CheckAccess() const;
		void VerifyAccess() const;
		
		void InvokeAsync(std::function<void()> task);
		void ProcessTasks();

		bool IsRunning() const;
		bool IsResizing() const;

		float DeltaTime() const { return deltaTime_; }
		float TotalTime() const { return totalTime_; }

		void Start();
		void Stop();

		void Tick();
		void BeginResize();

		std::chrono::milliseconds GetRemainingFrameTime(int targetFPS = 60);
	};
}
