
#include <cstdint>
#include <algorithm>
#include <string>
#include <vector>
#include <cwctype>
#include <cstring>

#include <terminality/Terminality.hpp>

using namespace terminality;

namespace
{
	class MenuPopupHost : public ControlBase
	{
		std::shared_ptr<Menu> menu_;

	public:
		explicit MenuPopupHost(std::shared_ptr<Menu> menu)
			: menu_(std::move(menu)) { }

		bool IsFocusable() const override { return true; }

		Size MeasureOverride(const Size& availableSize) override
		{
			return menu_->Measure(availableSize);
		}

		void ArrangeOverride(const Rect& contentRect) override
		{
			menu_->Arrange(contentRect);
		}

		void RenderOverride(RenderContext& context) override
		{
			menu_->Render(context);
		}

		bool OnKeyDown(InputEvent input) override
		{
			return menu_->OnKeyDown(input);
		}

		void OnGotFocus() override
		{
			menu_->OnGotFocus();
		}

		void OnLostFocus() override
		{
			menu_->OnLostFocus();
		}
	};
}

static std::wstring ToWString(const std::string& text)
{
	return std::wstring(text.begin(), text.end());
}

static wchar_t ToLower(wchar_t ch)
{
	if (ch >= L'A' && ch <= L'Z')
		return ch - L'A' + L'a';
	return std::towlower(ch);
}

static bool KeyMatchesAccessKey(InputKey key, wchar_t accessKey)
{
	if (accessKey == L'\0')
		return false;

	if (key < InputKey::A || key > InputKey::Z)
		return false;

	wchar_t keyChar = static_cast<wchar_t>(static_cast<int>(key));
	return ToLower(keyChar) == ToLower(accessKey);
}

std::wstring Menu::GetDisplayText(const std::wstring& text, wchar_t& accessKey, int32_t& accessKeyIndex) const
{
	std::wstring result;
	accessKey = L'\0';
	accessKeyIndex = -1;

	for (std::size_t i = 0; i < text.size(); ++i)
	{
		wchar_t ch = text[i];
		if (ch == L'&' && i + 1 < text.size())
		{
			wchar_t next = text[i + 1];
			if (next == L'&')
			{
				result.push_back(L'&');
				++i;
			}
			else if (accessKey == L'\0')
			{
				accessKey = next;
				accessKeyIndex = static_cast<int32_t>(result.size());
				result.push_back(next);
				++i;
			}
		}
		else
		{
			result.push_back(ch);
		}
	}

	return result;
}

void Menu::AddItem(const std::wstring& text, std::function<void()> action)
{
	items_.push_back(MenuItem{ text, L"", std::move(action), nullptr, false, true, false, false });
	InvalidateMeasure();
}

void Menu::AddItem(const std::string& text, std::function<void()> action)
{
	AddItem(ToWString(text), std::move(action));
}

void Menu::AddCheckableItem(const std::wstring& text, bool checked, std::function<void(bool)> action)
{
	std::size_t index = items_.size();

	MenuItem item;
	item.Text = text;
	item.IsEnabled = true;
	item.IsCheckable = true;
	item.IsChecked = checked;
	item.Action = [this, index, action]()
	{
		MenuItem& it = items_[index];
		it.IsChecked = !it.IsChecked;
		if (action != nullptr)
			action(it.IsChecked);
	};

	items_.push_back(std::move(item));
	InvalidateMeasure();
}

void Menu::AddSeparator()
{
	items_.push_back(MenuItem{ L"", L"", nullptr, nullptr, true, true, false, false });
	InvalidateMeasure();
}

void Menu::AddSubMenu(const std::wstring& text, std::shared_ptr<Menu> submenu)
{
	MenuItem item;
	item.Text = text;
	item.SubMenu = std::move(submenu);

	Menu* sub = item.SubMenu.get();
	sub->parentMenu_ = this;
	sub->onSubMenuAction_ = [this]()
	{
		CloseMenu();
	};

	items_.push_back(std::move(item));
	InvalidateMeasure();
}

void Menu::Clear()
{
	items_.clear();
	selectedIndex_ = 0;
	InvalidateMeasure();
}

void Menu::Open(Point position)
{
	if (items_.empty())
		return;

	HostApplication& host = HostApplication::Current();
	VisualTree& tree = VisualTree::Current();

	auto self = shared_from_this();
	auto popup = std::make_unique<MenuPopupHost>(self);
	popup->HorizontalAlignment = HorizontalAlign::Left;
	popup->VerticalAlignment = VerticalAlign::Top;
	popup->Margin = Thickness(position.X, position.Y, 0, 0);

	UILayer& layer = tree.PushLayer(std::move(popup));
	running_ = &layer.Running;

	FocusManager::Current().SetFocused(layer.RootNode.get());
	host.NestUILoop(layer);
	running_ = nullptr;
	tree.PopLayer();
}

void Menu::CloseMenu()
{
	if (running_ != nullptr)
		running_->store(false);
}

void Menu::OpenSubMenu(Menu* submenu)
{
	if (submenu == nullptr)
		return;

	Rect rect = GetArrangedRect();
	submenu->Open(Point(rect.X + rect.Width - 1, rect.Y + static_cast<int32_t>(selectedIndex_)));
}

void Menu::InvokeItem(MenuItem& item)
{
	if (!item.IsEnabled || item.IsSeparator)
		return;

	if (item.SubMenu != nullptr)
	{
		OpenSubMenu(item.SubMenu.get());
		return;
	}

	if (item.Action != nullptr)
		item.Action();

	if (parentMenu_ != nullptr && onSubMenuAction_)
		onSubMenuAction_();

	CloseMenu();
}

bool Menu::HandleAccessKey(InputKey key)
{
	for (std::size_t i = 0; i < items_.size(); ++i)
	{
		if (items_[i].IsSeparator)
			continue;

		wchar_t accessKey = L'\0';
		int32_t index = -1;
		GetDisplayText(items_[i].Text, accessKey, index);

		if (KeyMatchesAccessKey(key, accessKey))
		{
			selectedIndex_ = i;
			InvalidateVisual();
			InvokeItem(items_[i]);
			return true;
		}
	}

	return false;
}

void Menu::MoveSelection(int delta)
{
	if (items_.empty())
		return;

	std::size_t start = selectedIndex_;
	do
	{
		int next = static_cast<int>(selectedIndex_) + delta;
		if (next < 0)
			selectedIndex_ = items_.size() - 1;
		else if (next >= static_cast<int>(items_.size()))
			selectedIndex_ = 0;
		else
			selectedIndex_ = static_cast<std::size_t>(next);

		if (selectedIndex_ == start)
			break;
	} while (items_[selectedIndex_].IsSeparator || !items_[selectedIndex_].IsEnabled);

	InvalidateVisual();
}

void Menu::SelectNextMatch(wchar_t ch)
{
	if (items_.empty() || ch < 32)
		return;

	wchar_t target = ToLower(ch);
	std::size_t start = selectedIndex_;
	std::size_t i = (selectedIndex_ + 1) % items_.size();

	while (i != start)
	{
		if (!items_[i].IsSeparator && items_[i].IsEnabled)
		{
			wchar_t accessKey = L'\0';
			int32_t index = -1;
			GetDisplayText(items_[i].Text, accessKey, index);
			if (accessKey != L'\0' && ToLower(accessKey) == target)
			{
				selectedIndex_ = i;
				InvalidateVisual();
				return;
			}

			if (!items_[i].Text.empty() && ToLower(items_[i].Text[0]) == target)
			{
				selectedIndex_ = i;
				InvalidateVisual();
				return;
			}
		}

		i = (i + 1) % items_.size();
	}
}

bool Menu::OnKeyDown(InputEvent input)
{
	if (!IsEnabled)
		return ControlBase::OnKeyDown(input);

	if (input.Pressed && hasFlag(input.Modifier, InputModifier::Alt) &&
	    input.Key >= InputKey::A && input.Key <= InputKey::Z)
	{
		if (HandleAccessKey(input.Key))
			return true;
	}

	switch (input.Key)
	{
		case InputKey::UP:
		{
			MoveSelection(-1);
			return true;
		}

		case InputKey::DOWN:
		{
			MoveSelection(1);
			return true;
		}

		case InputKey::HOME:
		{
			selectedIndex_ = 0;
			MoveSelection(0);
			return true;
		}

		case InputKey::END:
		{
			selectedIndex_ = items_.size() - 1;
			MoveSelection(0);
			return true;
		}

		case InputKey::RETURN:
		case InputKey::SPACE:
		{
			if (selectedIndex_ < items_.size())
				InvokeItem(items_[selectedIndex_]);
			return true;
		}

		case InputKey::RIGHT:
		{
			if (selectedIndex_ < items_.size() && items_[selectedIndex_].SubMenu != nullptr)
			{
				OpenSubMenu(items_[selectedIndex_].SubMenu.get());
				return true;
			}
			break;
		}

		case InputKey::LEFT:
		case InputKey::ESCAPE:
		{
			CloseMenu();
			return true;
		}
	}

	if (input.Pressed && input.Key == InputKey::CHAR && input.Char >= 32)
	{
		SelectNextMatch(input.Char);
		return true;
	}

	return ControlBase::OnKeyDown(input);
}

void Menu::OnGotFocus()
{
	focused_ = true;
	InvalidateVisual();
}

void Menu::OnLostFocus()
{
	focused_ = false;
	InvalidateVisual();
}

Size Menu::MeasureOverride(const Size& availableSize)
{
	int32_t maxInteriorWidth = 0;
	for (const auto& item : items_)
	{
		if (item.IsSeparator)
		{
			maxInteriorWidth = std::max(maxInteriorWidth, 4);
			continue;
		}

		wchar_t accessKey = L'\0';
		int32_t accessIndex = -1;
		std::wstring text = GetDisplayText(item.Text, accessKey, accessIndex);

		int32_t width = 2; // left/right padding inside the item
		if (item.IsCheckable)
			width += 2;

		width += static_cast<int32_t>(text.length());

		if (!item.ShortcutText.empty())
			width += 2 + static_cast<int32_t>(item.ShortcutText.length());

		if (item.SubMenu != nullptr)
			width += 2;

		maxInteriorWidth = std::max(maxInteriorWidth, width);
	}

	int32_t width = maxInteriorWidth + 2; // left/right border
	int32_t height = static_cast<int32_t>(items_.size()) + 2; // top/bottom border

	if (availableSize.Width >= 0)
		width = std::min(width, availableSize.Width);

	if (availableSize.Height >= 0)
		height = std::min(height, availableSize.Height);

	return Size(width, height);
}

void Menu::ArrangeOverride(const Rect& /*contentRect*/)
{
}

void Menu::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	const Color fg = GetEffectiveForegroundColor();
	const Color bg = GetEffectiveBackgroundColor();
	const Color disabledFg = DisabledForegroundColor.Get();
	const Color disabledBg = DisabledBackgroundColor.Get();
	const Color lineFg = Color::DARK_GRAY;

	// Border
	for (int32_t x = 0; x < rect.Width; ++x)
	{
		context.SetCell(x, 0, L'\u2500', lineFg, bg);
		context.SetCell(x, rect.Height - 1, L'\u2500', lineFg, bg);
	}

	for (int32_t y = 0; y < rect.Height; ++y)
	{
		context.SetCell(0, y, L'\u2502', lineFg, bg);
		context.SetCell(rect.Width - 1, y, L'\u2502', lineFg, bg);
	}

	context.SetCell(0, 0, L'\u250C', lineFg, bg);
	context.SetCell(rect.Width - 1, 0, L'\u2510', lineFg, bg);
	context.SetCell(0, rect.Height - 1, L'\u2514', lineFg, bg);
	context.SetCell(rect.Width - 1, rect.Height - 1, L'\u2518', lineFg, bg);

	// Items
	for (std::size_t i = 0; i < items_.size(); ++i)
	{
		int32_t y = 1 + static_cast<int32_t>(i);
		if (y >= rect.Height - 1)
			break;

		const MenuItem& item = items_[i];
		bool selected = (i == selectedIndex_);
		bool enabled = item.IsEnabled && !item.IsSeparator;

		Color itemFg = selected ? FocusedForegroundColor.Get() : fg;
		Color itemBg = selected ? FocusedBackgroundColor.Get() : bg;

		if (!enabled && !selected)
		{
			itemFg = disabledFg;
			itemBg = disabledBg;
		}

		// Clear the row interior
		for (int32_t x = 1; x < rect.Width - 1; ++x)
			context.SetCell(x, y, L' ', itemFg, itemBg);

		if (item.IsSeparator)
		{
			for (int32_t x = 1; x < rect.Width - 1; ++x)
				context.SetCell(x, y, L'\u2500', lineFg, bg);
			continue;
		}

		int32_t x = 1;

		// Checkable prefix
		if (item.IsCheckable)
		{
			context.SetCell(x, y, item.IsChecked ? L'\u2713' : L' ', itemFg, itemBg);
			x += 2;
		}

		// Text with access-key highlight
		wchar_t accessKey = L'\0';
		int32_t accessIndex = -1;
		std::wstring text = GetDisplayText(item.Text, accessKey, accessIndex);

		for (std::size_t ti = 0; ti < text.size() && x < rect.Width - 1; ++ti, ++x)
		{
			bool isAccess = (static_cast<int32_t>(ti) == accessIndex);
			Color charFg = isAccess ? (selected ? bg : FocusedForegroundColor.Get()) : itemFg;
			Color charBg = isAccess ? (selected ? fg : FocusedBackgroundColor.Get()) : itemBg;
			context.SetCell(x, y, text[ti], charFg, charBg);
		}

		// Shortcut text, right aligned
		if (!item.ShortcutText.empty())
		{
			int32_t shortcutX = rect.Width - 1 - 1 - static_cast<int32_t>(item.ShortcutText.length());
			if (shortcutX > x)
			{
				for (std::size_t si = 0; si < item.ShortcutText.size() && shortcutX < rect.Width - 1; ++si, ++shortcutX)
					context.SetCell(shortcutX, y, item.ShortcutText[si], itemFg, itemBg);
			}
		}

		// Submenu arrow
		if (item.SubMenu != nullptr && rect.Width - 2 > x)
			context.SetCell(rect.Width - 2, y, L'\u25B6', itemFg, itemBg);
	}
}
