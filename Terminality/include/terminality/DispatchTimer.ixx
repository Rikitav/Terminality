export module terminality:DispatchTimer;

import std;
import std.compat;
import :EventSignal;

export namespace terminality
{
	class DispatchTimer
	{
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
		EventSignal<float> TickEvent;
		EventSignal<> ResizeFinishedEvent;

		static DispatchTimer& Current();

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
