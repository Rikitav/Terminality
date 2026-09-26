#pragma once

#include <cstdint>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <functional>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include <terminality/Framework/Event.hpp>

namespace terminality
{
	class DispatchTimer
	{
		struct TimerEntry
		{
			std::function<void()> callback;
			std::chrono::milliseconds period { 0 };
			bool repeat = false;
			float nextFire = 0.0f; // DispatchTimer::TotalTime() domain (seconds)
		};

		std::optional<std::thread::id> uiThreadId_;
		std::mutex mutex_;
		std::vector<std::function<void()>> tasks_;

		std::unordered_map<std::uint64_t, TimerEntry> timers_;
		std::uint64_t nextTimerId_ = 1;

		float deltaTime_ = 0.0f;
		float totalTime_ = 0.0f;
		std::atomic<bool> running_{ false };

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

		void CancelTimer(std::uint64_t id);

	public:
		class TimerHandle
		{
			friend class DispatchTimer;
			std::uint64_t id_ = 0;

			explicit TimerHandle(std::uint64_t id) : id_(id) { }

		public:
			TimerHandle() = default;
			~TimerHandle();

			TimerHandle(TimerHandle&& other) noexcept : id_(std::exchange(other.id_, 0)) { }
			TimerHandle& operator=(TimerHandle&& other) noexcept;
			TimerHandle(const TimerHandle&) = delete;
			TimerHandle& operator=(const TimerHandle&) = delete;

			void Cancel();
			[[nodiscard]] bool IsActive() const;
		};

		Event<float> TickEvent;
		Event<> ResizeFinishedEvent;

		static DispatchTimer& Current();

		void SetUIThread();
		bool CheckAccess() const;
		void VerifyAccess() const;
		
		void InvokeAsync(std::function<void()> task);
		void ProcessTasks();

		[[nodiscard]] TimerHandle SetInterval(std::chrono::milliseconds interval, std::function<void()> callback);
		[[nodiscard]] TimerHandle SetTimeout(std::chrono::milliseconds delay, std::function<void()> callback);

		bool IsRunning() const;
		bool IsResizing() const;

		float DeltaTime() const;
		float TotalTime() const;

		void Start();
		void Stop();

		void Tick();
		void BeginResize();

		std::chrono::milliseconds GetRemainingFrameTime(int targetFPS = 60);
	};
}
