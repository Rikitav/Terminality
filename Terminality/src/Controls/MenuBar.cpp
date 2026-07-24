
#include <cstdint>
#include <algorithm>
#include <string>
#include <vector>
#include <cwctype>
#include <cstring>

#include <terminality/Controls/MenuBar.hpp>

using namespace terminality;

namespace
{
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

	static std::wstring GetDisplayText(const std::wstring& text, wchar_t& accessKey)
	{
		std::wstring result;
		accessKey = L'\0';

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
}

void MenuBar::AddMenu(const std::wstring& header, std::shared_ptr<Menu> menu)
{
	items_.push_back(BarItem{ header, std::move(menu) });
	InvalidateMeasure();
}

void MenuBar::AddMenu(const std::string& header, std::shared_ptr<Menu> menu)
{
	AddMenu(ToWString(header), std::move(menu));
}

void MenuBar::AddMenu(const std::wstring& header, std::unique_ptr<Menu> menu)
{
	AddMenu(header, std::shared_ptr<Menu>(std::move(menu)));
}

void MenuBar::AddMenu(const std::string& header, std::unique_ptr<Menu> menu)
{
	AddMenu(ToWString(header), std::shared_ptr<Menu>(std::move(menu)));
}

void MenuBar::OpenSelectedMenu()
{
	if (selectedIndex_ >= items_.size())
		return;

	if (items_[selectedIndex_].Menu == nullptr)
		return;

	Rect rect = GetArrangedRect();

	int32_t x = rect.X;
	for (std::size_t i = 0; i < selectedIndex_; ++i)
	{
		wchar_t accessKey = L'\0';
		std::wstring text = GetDisplayText(items_[i].Header, accessKey);
		x += 3 + static_cast<int32_t>(text.length()); // 2 spaces padding + 1 separator
	}

	items_[selectedIndex_].Menu->Open(Point(x, rect.Y + 1));
}

bool MenuBar::HandleAccessKey(InputKey key)
{
	for (std::size_t i = 0; i < items_.size(); ++i)
	{
		wchar_t accessKey = L'\0';
		GetDisplayText(items_[i].Header, accessKey);

		if (KeyMatchesAccessKey(key, accessKey))
		{
			selectedIndex_ = i;
			InvalidateVisual();
			OpenSelectedMenu();
			return true;
		}
	}

	return false;
}

void MenuBar::MoveSelection(int delta)
{
	if (items_.empty())
		return;

	int next = static_cast<int>(selectedIndex_) + delta;
	if (next < 0)
		selectedIndex_ = items_.size() - 1;
	else if (next >= static_cast<int>(items_.size()))
		selectedIndex_ = 0;
	else
		selectedIndex_ = static_cast<std::size_t>(next);

	InvalidateVisual();
}

bool MenuBar::OnKeyDown(InputEvent input)
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
		case InputKey::LEFT:
		{
			MoveSelection(-1);
			return true;
		}

		case InputKey::RIGHT:
		{
			MoveSelection(1);
			return true;
		}

		case InputKey::DOWN:
		case InputKey::RETURN:
		case InputKey::SPACE:
		{
			OpenSelectedMenu();
			return true;
		}
	}

	if (input.Pressed && input.Key == InputKey::CHAR && input.Char >= 32)
	{
		wchar_t target = ToLower(input.Char);
		for (std::size_t i = 0; i < items_.size(); ++i)
		{
			wchar_t accessKey = L'\0';
			GetDisplayText(items_[i].Header, accessKey);
			if (accessKey != L'\0' && ToLower(accessKey) == target)
			{
				selectedIndex_ = i;
				OpenSelectedMenu();
				return true;
			}
		}
	}

	return ControlBase::OnKeyDown(input);
}

void MenuBar::OnGotFocus()
{
	focused_ = true;
	InvalidateVisual();
}

void MenuBar::OnLostFocus()
{
	focused_ = false;
	InvalidateVisual();
}

Size MenuBar::MeasureOverride(const Size& availableSize)
{
	int32_t width = 0;
	for (const auto& item : items_)
	{
		wchar_t accessKey = L'\0';
		std::wstring text = GetDisplayText(item.Header, accessKey);
		width += 3 + static_cast<int32_t>(text.length());
	}

	if (!items_.empty())
		width -= 1;

	int32_t height = 1;

	if (availableSize.Width >= 0)
		width = std::min(width, availableSize.Width);

	if (availableSize.Height >= 0)
		height = std::min(height, availableSize.Height);

	return Size(width, height);
}

void MenuBar::ArrangeOverride(const Rect& /*contentRect*/)
{
}

void MenuBar::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	const Color fg = GetEffectiveForegroundColor();
	const Color bg = GetEffectiveBackgroundColor();

	for (int32_t x = 0; x < rect.Width; ++x)
		context.SetCell(x, 0, L' ', fg, bg);

	int32_t x = 0;
	for (std::size_t i = 0; i < items_.size(); ++i)
	{
		const auto& item = items_[i];
		bool selected = (i == selectedIndex_);

		wchar_t accessKey = L'\0';
		std::wstring text = GetDisplayText(item.Header, accessKey);

		Color itemFg = selected ? FocusedForegroundColor.Get() : fg;
		Color itemBg = selected ? FocusedBackgroundColor.Get() : bg;

		context.SetCell(x, 0, L' ', itemFg, itemBg);
		++x;

		for (std::size_t ti = 0; ti < text.size() && x < rect.Width; ++ti, ++x)
		{
			bool isAccess = (text[ti] == accessKey && accessKey != L'\0');
			Color charFg = isAccess ? (selected ? bg : FocusedForegroundColor.Get()) : itemFg;
			Color charBg = isAccess ? (selected ? fg : FocusedBackgroundColor.Get()) : itemBg;
			context.SetCell(x, 0, text[ti], charFg, charBg);
		}

		if (x < rect.Width)
		{
			context.SetCell(x, 0, L' ', itemFg, itemBg);
			++x;
		}
	}
}
