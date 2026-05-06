module terminality;

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

	VisualTree& tree = VisualTree::Current();
	FocusManager& focus = FocusManager::Current();

	tree.SetRoot(std::move(rootNode));
	focus.SetFocused(tree.Root());

	const Size initViewport = HostBackend::QueryViewportSize();
	renderBuffer_.Resize(static_cast<uint32_t>(initViewport.Width), static_cast<uint32_t>(initViewport.Height));

	Running.store(true);
	while (Running.load())
	{
		const Size viewport = HostBackend::QueryViewportSize();
		if (viewport.Height != renderBuffer_.Height() || viewport.Width != renderBuffer_.Width())
		{
			renderBuffer_.Resize(static_cast<uint32_t>(viewport.Width), static_cast<uint32_t>(viewport.Height));
			tree.Root()->InvalidateVisual();
		}

		const InputEvent evt = HostBackend::PollInput(std::chrono::milliseconds(16));
		if (evt.Key != InputKey::None)
		{
			if (evt.Key == InputKey::ESCAPE)
			{
				Running.store(false);
				break;
			}

			VisualTreeNode* focused = focus.GetFocused();
			bool success = false;
			while (!success)
			{
				if (focused == nullptr)
					break;

				success = evt.Pressed
					? focused->OnKeyDown(evt)
					: focused->OnKeyUp(evt);

				focused = focused->GetParent();
			}
		}

		tree.RunLayout(viewport);
		if (tree.HasDirtyVisual())
		{
			tree.Render(renderBuffer_);
			renderBuffer_.DiffRender(std::wcout);
		}
	}
}

void HostApplication::RequestStop()
{
	Running.store(false);
}
