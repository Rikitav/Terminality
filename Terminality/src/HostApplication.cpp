module terminality;

import std;
import std.compat;

using namespace terminality;

void HostApplication::RunUILoop()
{
	if (visualTree_->Root() == nullptr)
		return;

	Running.store(true);
	bool firstRender = true;

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
			if (!firstRender)
			{
				renderBuffer_.DiffRender(std::wcout);
				continue;
			}

			renderBuffer_.BulkRender(std::wcout);
			firstRender = false;
		}
	}
}

void HostApplication::RequestStop()
{
	Running.store(false);
}
