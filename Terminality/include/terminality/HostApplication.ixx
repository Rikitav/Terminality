export module terminality:Host;

import std;
import std.compat;
import :Geometry;
import :InputEvent;
import :VisualTree;
import :VisualTreeNode;
import :RenderBuffer;
import :FocusManager;

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

		std::unique_ptr<VisualTreeNode> rootNode;
		RenderBuffer renderBuffer_{ 1, 1 };
		std::atomic<bool> Running = false;

		HostApplication() = default;
		
	public:
		static HostApplication& Current();
		static bool IsUiThread();

		void SetRoot(std::unique_ptr<VisualTreeNode> root);

		void EnterTerminal();
		void ExitTerminal();

		void RunUILoop();
		void RequestStop();
	};
}
