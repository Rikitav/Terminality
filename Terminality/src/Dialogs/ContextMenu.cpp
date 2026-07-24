
#include <functional>
#include <string>
#include <cstdint>
#include <atomic>
#include <memory>
#include <optional>

#include <terminality/Dialogs/ContextMenu.hpp>

#include <terminality/Controls/Menu.hpp>

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
	if (items_.empty())
		return;

	auto menu = std::shared_ptr<Menu>(init<Menu>([&](Menu* menu)
	{
		for (const ContextMenuItem& item : items_)
			menu->AddItem(item.Text, item.Action);
	}).release());

	menu->Open(position);
}
