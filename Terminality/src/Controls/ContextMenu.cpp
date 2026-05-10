module terminality;

import std;
import std.compat;

using namespace terminality;

void ContextMenu::AddItem(const std::wstring& text, std::function<void()> action)
{
	items_.push_back({ text, std::move(action) });
}

void ContextMenu::Clear()
{
	items_.clear();
}

void ContextMenu::Open(Point position)
{
	HostApplication& host = HostApplication::Current();
	VisualTree& tree = VisualTree::Current();

	std::atomic<bool>* running = nullptr;

	if (items_.empty())
		return;

	auto ctxMenuBody = init<Border>([&](Border* ctxMenuBody)
	{
		ctxMenuBody->HorizontalAlignment = HorizaontalAllign::Left;
		ctxMenuBody->VerticalAlignment = VerticalAlign::Top;
		ctxMenuBody->Margin = Thickness(position.X, position.Y, 0, 0);
		ctxMenuBody->BorderColor = Color::YELLOW;
		ctxMenuBody->FocusedBorderColor = Color::YELLOW;

		ctxMenuBody->Content = init<StackPanel>([&](StackPanel* panel)
		{
			panel->HorizontalAlignment = HorizaontalAllign::Stretch;
			panel->VerticalAlignment = VerticalAlign::Stretch;

			for (const ContextMenuItem& item : items_)
			{
				panel->AddChild(init<Button>([&](Button* itemButton)
				{
					itemButton->Text = item.Text;
					itemButton->HorizontalAlignment = HorizaontalAllign::Stretch;
					itemButton->Clicked += [&running, item]()
					{
						if (item.Action != nullptr)
							item.Action();

						running->store(false);
					};
				}));
			}
		});
	});

	UILayer& layer = tree.PushLayer(std::move(ctxMenuBody));
	running = &layer.Running;
	
	host.NestUILoop(layer);
	tree.PopLayer();
}