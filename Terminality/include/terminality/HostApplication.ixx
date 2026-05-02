export module terminality:Host;

import std;
import std.compat;
import :Geometry;
import :InputEvent;
import :VisualTree;
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
		VisualTree* visualTree_;
		FocusManager* focusManager_;
		RenderBuffer renderBuffer_{ 1, 1 };
		std::atomic<bool> Running = false;

		HostApplication();

	public:
		static HostApplication* Current();
		VisualTree& Tree() { return *visualTree_; }
		FocusManager& Focus() { return *focusManager_; }
		RenderBuffer& Buffer() { return renderBuffer_; }

		void EnterTerminal();
		void ExitTerminal();

		void RunUILoop();
		void RequestStop();
	};
}
