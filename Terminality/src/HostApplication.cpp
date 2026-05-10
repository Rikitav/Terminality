module terminality;

//#include <chrono>

import std;
import std.compat;

using namespace terminality;

std::optional<std::thread::id> HostApplication::uiThreadId;

bool HostApplication::IsUiThread()
{
	if (uiThreadId.has_value())
		return false;

	return std::this_thread::get_id() == uiThreadId;
}

HostApplication& HostApplication::Current()
{
	static HostApplication app;
	return app;
}

void HostApplication::SetRoot(std::unique_ptr<VisualTreeNode> root)
{
	rootNode = std::move(root);
}

void HostApplication::RunUILoop()
{
	if (uiThreadId.has_value())
		throw std::runtime_error("UI loop thread was already start somewhere else");

	uiThreadId = std::this_thread::get_id();
	if (rootNode == nullptr)
		return;

	DispatchTimer& timer = DispatchTimer::Current();
	VisualTree& tree = VisualTree::Current();
	UILayer& layer = tree.PushLayer(std::move(rootNode));

	timer.Start();
	NestUILoop(layer);
	timer.Stop();
}

void HostApplication::NestUILoop(UILayer& layer)
{
	DispatchTimer& timer = DispatchTimer::Current();
	VisualTree& tree = VisualTree::Current();

	const Size initViewport = HostBackend::QueryViewportSize();
	renderBuffer_.Resize(static_cast<uint32_t>(initViewport.Width), static_cast<uint32_t>(initViewport.Height));

	// Подписываемся на завершение ресайза для инвалидации дерева
	auto resizeConn = timer.ResizeFinishedEvent.Connect([&tree]()
	{
		if (tree.Root())
		{
			tree.Root()->InvalidateMeasure();
			tree.Root()->InvalidateVisual();
		}
	});

	layer.Running.store(true);
	while (layer.Running.load() && timer.IsRunning())
	{
		timer.Tick();

		const Size viewport = HostBackend::QueryViewportSize();
		if (viewport.Height != renderBuffer_.Height() || viewport.Width != renderBuffer_.Width())
		{
			renderBuffer_.Resize(static_cast<uint32_t>(viewport.Width), static_cast<uint32_t>(viewport.Height));
			timer.BeginResize();
		}

		if (!timer.IsResizing())
		{
			tree.RunLayout(viewport);
			if (tree.HasDirtyVisual())
			{
				tree.Render(renderBuffer_);
				renderBuffer_.DiffRender(std::wcout);
			}
		}

		const InputEvent evt = HostBackend::PollInput(timer.GetRemainingFrameTime(60));
		if (evt.Key != InputKey::None)
		{
			if (evt.Key == InputKey::ESCAPE)
			{
				layer.Running.store(false);
				break;
			}

			FocusManager& focus = tree.GetFocusManager();
			VisualTreeNode* focused = focus.GetFocused();

			bool success = false;
			while (!success && focused != nullptr)
			{
				success = evt.Pressed
					? focused->OnKeyDown(evt)
					: focused->OnKeyUp(evt);

				focused = focused->GetParent();
			}

			if (!timer.IsResizing())
			{
				tree.RunLayout(viewport);
				if (tree.HasDirtyVisual())
				{
					tree.Render(renderBuffer_);
					renderBuffer_.DiffRender(std::wcout);
					//renderBuffer_.BulkRender(std::wcout);
				}
			}
		}
	}
}

void HostApplication::RequestStop()
{
	DispatchTimer& timer = DispatchTimer::Current();
	timer.Stop();
}
