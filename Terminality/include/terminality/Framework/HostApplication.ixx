export module terminality:Host;

import <optional>;
import <chrono>;
import <thread>;
import <memory>;

import :Geometry;
import :InputEvent;
import :VisualTree;
import :VisualTreeNode;
import :RenderBuffer;
import :FocusManager;
import :Event;

export namespace terminality
{
	class HostBackend
	{
	public:
		static Size QueryViewportSize();
		static InputEvent PollInput(std::chrono::milliseconds timeout);
	};

	class HostApplication
	{
		static std::optional<std::thread::id> uiThreadId;

		bool isResizing_ = false;
		float resizeDebounceTimer_ = 0.0f;
		float RESIZE_DELAY = 0.1f;

		RenderBuffer renderBuffer_{ 1, 1 };

		HostApplication() = default;
		HostApplication(const HostApplication&) = delete;
		HostApplication& operator=(const HostApplication&) = delete;

	public:
		static HostApplication& Current();
		static bool IsUiThread();

		void EnterTerminal();
		void ExitTerminal();

		void RunUILoop(std::unique_ptr<VisualTreeNode> root);
		void NestUILoop(UILayer& layer);
		void RequestStop();
	};
}
