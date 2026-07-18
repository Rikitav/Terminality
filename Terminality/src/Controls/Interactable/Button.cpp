
#include <cstdint>
#include <algorithm>
#include <string>
#include <cwctype>
#include <cstring>

#include <terminality/Terminality.hpp>

using namespace terminality;

static wchar_t ToLowerKey(wchar_t ch)
{
	if (ch >= L'A' && ch <= L'Z')
		return ch - L'A' + L'a';
	return ch;
}

static bool KeyMatchesAccessKey(InputKey key, wchar_t accessKey)
{
	if (accessKey == L'\0')
		return false;

	// A-Z InputKey values are 0x41-0x5A.
	if (key < InputKey::A || key > InputKey::Z)
		return false;

	wchar_t keyChar = static_cast<wchar_t>(static_cast<int>(key));
	return ToLowerKey(keyChar) == ToLowerKey(accessKey);
}

void Button::UpdateAccessKey()
{
	displayText_.clear();
	accessKey_ = L'\0';
	accessKeyIndex_ = -1;

	const std::wstring& text = Text.Get();
	for (std::size_t i = 0; i < text.size(); ++i)
	{
		wchar_t ch = text[i];
		if (ch == L'&' && i + 1 < text.size())
		{
			wchar_t next = text[i + 1];
			if (next == L'&')
			{
				displayText_.push_back(L'&');
				++i;
			}
			else if (accessKey_ == L'\0')
			{
				accessKey_ = next;
				accessKeyIndex_ = static_cast<int32_t>(displayText_.size());
			}
		}
		else
		{
			displayText_.push_back(ch);
		}
	}
}

void Button::Click()
{
	if (!IsEnabled)
		return;

	isPressed_ = true;
	InvalidateVisual();
	Clicked.Emit();
	isPressed_ = false;
	InvalidateVisual();
}

void Button::OnPropertyChanged(const char* propertyName)
{
	if (std::strcmp(propertyName, "Text") == 0)
		UpdateAccessKey();

	ControlBase::OnPropertyChanged(propertyName);
}

bool Button::OnKeyDown(InputEvent input)
{
	if (!IsEnabled)
		return ControlBase::OnKeyDown(input);

	switch (input.Key)
	{
		case InputKey::RETURN:
		case InputKey::SPACE:
		{
			isPressed_ = true;
			InvalidateVisual();
			Clicked.Emit();
			return true;
		}
	}

	return ControlBase::OnKeyDown(input);
}

bool Button::OnKeyUp(InputEvent input)
{
	if (!IsEnabled)
		return ControlBase::OnKeyUp(input);

	switch (input.Key)
	{
		case InputKey::RETURN:
		case InputKey::SPACE:
		{
			isPressed_ = false;
			InvalidateVisual();
			return true;
		}
	}

	return ControlBase::OnKeyUp(input);
}

void Button::OnLostFocus()
{
	focused_ = false;
	isPressed_ = false;
	InvalidateVisual();
}

Size Button::MeasureOverride(const Size& availableSize)
{
	int32_t contentWidth = static_cast<int32_t>(displayText_.size()) + 6;
	int32_t width = availableSize.Width >= 0 ? std::min(availableSize.Width, contentWidth) : contentWidth;
	int32_t height = availableSize.Height >= 0 ? std::min(availableSize.Height, 1) : 1;
	return Size(width, height);
}

void Button::ArrangeOverride(const Rect& contentRect)
{
	// bleh U_U
	return;
}

void Button::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();

	wchar_t leftBracket = IsDefault ? L'<' : L'[';
	wchar_t rightBracket = IsDefault ? L'>' : L']';

	std::wstring line;
	line.reserve(displayText_.size() + 6);
	line += leftBracket;
	line += L"  ";
	line += displayText_;
	line += L"  ";
	line += rightBracket;

	Color fore = ForegroundColor;
	Color back = BackgroundColor;

	if (!IsEnabled)
	{
		fore = DisabledForegroundColor;
		back = DisabledBackgroundColor;
	}
	else if (focused_)
	{
		if (isPressed_)
		{
			fore = PressedForegroundColor;
			back = PressedBackgroundColor;
		}
		else
		{
			fore = FocusedForegroundColor;
			back = FocusedBackgroundColor;
		}
	}

	int32_t x = 0;
	for (std::size_t i = 0; i < line.size(); ++i)
	{
		if (x >= rect.Width)
			break;

		Color cellFore = fore;
		Color cellBack = back;

		// The access key sits inside the display text, offset by the two leading characters/brackets.
		if (IsEnabled && accessKeyIndex_ >= 0 &&
		    static_cast<int32_t>(i) == accessKeyIndex_ + 3)
		{
			if (isPressed_)
			{
				cellFore = PressedForegroundColor;
				cellBack = PressedBackgroundColor;
			}
			else if (focused_)
			{
				// Inverse the focused colors to make the access key pop.
				cellFore = back;
				cellBack = fore;
			}
			else
			{
				cellFore = AccessKeyForegroundColor;
				cellBack = AccessKeyBackgroundColor;
			}
		}

		context.SetCell(x, 0, line[i], cellFore, cellBack);
		++x;
	}
}
