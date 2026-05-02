module terminality;

import std;
import std.compat;

using namespace terminality;

HostApplication::HostApplication()
{
	focusManager_ = &FocusManager::Current();
	visualTree_ = new VisualTree();
}

HostApplication* HostApplication::Current()
{
	static HostApplication app;
	return &app;
}

void HostApplication::RunUILoop()
{
	if (visualTree_->Root() == nullptr)
		return;

	const Size initViewport = HostBackend::QueryViewportSize();
	renderBuffer_.Resize(static_cast<uint32_t>(initViewport.Width), static_cast<uint32_t>(initViewport.Height));

	Running.store(true);
	while (Running.load())
	{
		const Size viewport = HostBackend::QueryViewportSize();
		if (viewport.Height != renderBuffer_.Height() || viewport.Width != renderBuffer_.Width())
		{
			renderBuffer_.Resize(static_cast<uint32_t>(viewport.Width), static_cast<uint32_t>(viewport.Height));
			visualTree_->Root()->InvalidateVisual();
		}

		const InputEvent evt = HostBackend::PollInput(std::chrono::milliseconds(16));
		if (evt.Key != InputKey::None)
		{
			if (evt.Key == InputKey::ESCAPE)
			{
				Running.store(false);
				break;
			}

			VisualTreeNode* focused = focusManager_->GetFocused();
			if (focused != nullptr)
			{
				if (evt.Pressed)
					focused->OnKeyDown(evt);
				else
					focused->OnKeyUp(evt);
			}
		}

		visualTree_->RunLayout(viewport);
		if (visualTree_->HasDirtyVisual())
		{
			visualTree_->Render(renderBuffer_);
			renderBuffer_.DiffRender(std::wcout);
		}
	}
}

void HostApplication::RequestStop()
{
	Running.store(false);
}
