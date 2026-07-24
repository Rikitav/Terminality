#pragma once

#include <optional>
#include <chrono>
#include <thread>
#include <memory>
#include <string>

#include <terminality/Core/Geometry.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Framework/VisualTree.hpp>
#include <terminality/Framework/VisualTreeNode.hpp>
#include <terminality/Engine/RenderBuffer.hpp>
#include <terminality/Engine/FocusManager.hpp>
#include <terminality/Framework/Event.hpp>

namespace terminality
{
	class HostBackend
	{
	public:
		static Size QueryViewportSize();
		static InputEvent PollInput(std::chrono::milliseconds timeout);
	};

	class HostApplication
	{
		bool isResizing_ = false;
		float resizeDebounceTimer_ = 0.0f;
		float RESIZE_DELAY = 0.1f;

		RenderBuffer renderBuffer_{ 1, 1 };

		HostApplication() = default;
		HostApplication(const HostApplication&) = delete;
		HostApplication& operator=(const HostApplication&) = delete;

	public:
		static HostApplication& Current();

		void EnterTerminal();
		void ExitTerminal();

		void RunUILoop(std::unique_ptr<VisualTreeNode> root);
		void NestUILoop(UILayer& layer);
		void RequestStop();
	};

#ifdef _WIN32
	void AlertAsync(const std::wstring& text, const std::wstring& title);
#endif // _WIN32
}
