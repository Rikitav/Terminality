
#include <cstdint>
#include <algorithm>
#include <vector>
#include <string>
#include <cstring>

#include <terminality/Terminality.hpp>

using namespace terminality;

TextBox::TextBox()
{
	isTabStop_ = true;
	FocusedBackgroundColor = BackgroundColor;
	FocusedForegroundColor = ForegroundColor;
}

void TextBox::OnPropertyChanged(const char* propertyName)
{
	if (std::strcmp(propertyName, "Text") == 0)
	{
		std::wstring text = Text.Get();
		if (MaxLength >= 0 && text.size() > static_cast<std::size_t>(MaxLength.Get()))
		{
			Text = text.substr(0, static_cast<std::size_t>(MaxLength.Get()));
			ControlBase::OnPropertyChanged(propertyName);
			return;
		}

		cursorPosition_ = std::min(cursorPosition_, text.length());
		TextChanged.Emit();
	}

	ControlBase::OnPropertyChanged(propertyName);
}

Size TextBox::MeasureOverride(const Size& availableSize)
{
	std::vector<int32_t> lines = TextHelper::MeasureLines(Text, availableSize.Width, TextWrapping);

	int32_t maxWidth = 0;
	for (const auto& line : lines)
		maxWidth = std::max(maxWidth, line);

	int32_t width = availableSize.Width >= 0 ? std::clamp(maxWidth + 1, 0, availableSize.Width) : maxWidth + 1;
	int32_t desiredHeight = std::max<int32_t>(1, static_cast<int32_t>(lines.size()));
	int32_t height = availableSize.Height >= 0 ? std::min(desiredHeight, availableSize.Height) : desiredHeight;

	return Size(width, height);
}

void TextBox::ArrangeOverride(const Rect& contentRect)
{
	return;
}

static wchar_t GetDisplayChar(wchar_t ch, wchar_t passwordChar)
{
	if (passwordChar != L'\0' && ch != L'\n')
		return passwordChar;
	return ch;
}

static std::wstring GetDisplayLine(const std::wstring& line, wchar_t passwordChar)
{
	if (passwordChar == L'\0')
		return line;

	std::wstring result;
	result.reserve(line.size());
	for (wchar_t ch : line)
		result.push_back(GetDisplayChar(ch, passwordChar));
	return result;
}

void TextBox::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();

	const bool isPlaceholder = Text->empty() && !PlaceholderText->empty() && !focused_;
	Color fore = isPlaceholder ? PlaceholderForegroundColor : GetEffectiveForegroundColor();
	Color back = isPlaceholder ? PlaceholderBackgroundColor : GetEffectiveBackgroundColor();

	for (int32_t y = 0; y < rect.Height; ++y)
	{
		for (int32_t x = 0; x < rect.Width; ++x)
		{
			context.SetCell(x, y, L' ', fore, back);
		}
	}

	const std::wstring& sourceText = isPlaceholder ? PlaceholderText.Get() : Text.Get();
	const wchar_t passChar = PasswordChar;

	std::vector<LineInfo> lines = TextHelper::GetLines(sourceText, rect.Width, TextWrapping.Get());

	int32_t cursorY = 0;
	int32_t cursorX = 0;
	bool cursorFound = false;

	for (std::size_t y = 0; y < lines.size(); ++y)
	{
		const auto& line = lines[y];
		if (!cursorFound && cursorPosition_ >= line.StartIndex && (y + 1 == lines.size() || cursorPosition_ < lines[y + 1].StartIndex))
		{
			cursorY = static_cast<int32_t>(y);
			cursorX = static_cast<int32_t>(cursorPosition_ - line.StartIndex);
			cursorFound = true;
			break;
		}
	}

	int32_t viewWidth = rect.Width;
	int32_t offset = 0;

	if (TextWrapping == TextWrap::NoWrap)
	{
		if (cursorX >= viewWidth)
		{
			offset = cursorX - viewWidth + 1;
		}
	}

	cursorX -= offset;

	for (int32_t y = 0; y < std::min(rect.Height, static_cast<int32_t>(lines.size())); ++y)
	{
		const auto& line = lines[y];
		int32_t xOffset = 0;

		std::wstring visibleText = GetDisplayLine(line.Text, passChar);
		if (offset > 0 && offset < static_cast<int32_t>(visibleText.length()))
			visibleText = visibleText.substr(offset);
		else if (offset >= static_cast<int32_t>(visibleText.length()))
			visibleText = L"";

		int32_t textLen = static_cast<int32_t>(visibleText.length());
		if (TextAlignment.Get() == terminality::TextAlign::Center)
		{
			xOffset = std::max(0, (rect.Width - textLen) / 2);
		}
		else if (TextAlignment.Get() == terminality::TextAlign::Right)
		{
			xOffset = std::max(0, rect.Width - textLen);
		}

		if (y == cursorY)
		{
			cursorX += xOffset;
		}

		int32_t renderWidth = std::min(rect.Width - xOffset, textLen);
		if (renderWidth > 0)
		{
			context.RenderText(Point(xOffset, y), visibleText.substr(0, renderWidth), fore, back, false);
		}
	}

	if (focused_ && cursorY < rect.Height && cursorX < rect.Width && cursorX >= 0 && !isPlaceholder)
	{
		wchar_t cursorChar = L' ';
		if (cursorPosition_ < Text.Get().length())
		{
			wchar_t realChar = Text.Get()[cursorPosition_];
			cursorChar = GetDisplayChar(realChar, passChar);
		}

		context.SetCell(cursorX, cursorY, cursorChar, back, fore);
	}
}

bool TextBox::OnKeyDown(InputEvent input)
{
	if (!IsEnabled)
		return ControlBase::OnKeyDown(input);

	if (ControlBase::OnKeyDown(input))
		return true;

	std::wstring currentText = Text.Get();
	const bool canModify = !IsReadOnly;
	const bool canInsert = canModify && (MaxLength < 0 || static_cast<int32_t>(currentText.size()) < MaxLength.Get());

	auto applyTextChange = [&]()
	{
		Text = currentText;
		InvalidateMeasure();
		InvalidateVisual();
	};

	auto applyCursorMove = [&]()
	{
		InvalidateVisual();
	};

	auto getLineIndex = [&](const std::vector<LineInfo>& lines) -> size_t
	{
		for (std::size_t i = 0; i < lines.size(); ++i)
		{
			if (cursorPosition_ >= lines[i].StartIndex && (i + 1 == lines.size() || cursorPosition_ < lines[i + 1].StartIndex))
				return i;
		}

		return 0;
	};

	switch (input.Key)
	{
		default:
		{
			if (input.Char == L'\0' || input.Char < 32)
				return false;

			if (!canInsert)
				return true;

			currentText.insert(cursorPosition_++, 1, input.Char);
			applyTextChange();
			return true;
		}

		case InputKey::SPACE:
		{
			if (!canInsert)
				return true;

			currentText.insert(cursorPosition_++, 1, L' ');
			applyTextChange();
			return true;
		}

		case InputKey::RETURN:
		{
			if (!AcceptsReturn.Get())
				return true;

			if (!canInsert)
				return true;

			currentText.insert(cursorPosition_++, 1, L'\n');
			applyTextChange();
			return true;
		}

		case InputKey::UP:
		{
			if (!AcceptsReturn.Get() && TextWrapping.Get() == terminality::TextWrap::NoWrap)
			{
				PopFocus(Direction::Up, input.Modifier);
				return true;
			}

			auto lines = TextHelper::GetLines(currentText, arrangedRect_.Width, TextWrapping.Get());
			size_t lineIdx = getLineIndex(lines);

			if (lineIdx == 0)
			{
				PopFocus(Direction::Up, input.Modifier);
				return true;
			}

			size_t col = cursorPosition_ - lines[lineIdx].StartIndex;
			size_t prevLen = lines[lineIdx - 1].Text.length();

			if (prevLen > 0 && lines[lineIdx - 1].Text.back() == L'\n')
				prevLen--;

			cursorPosition_ = lines[lineIdx - 1].StartIndex + std::min(col, prevLen);
			applyCursorMove();
			return true;
		}

		case InputKey::DOWN:
		{
			if (!AcceptsReturn.Get() && TextWrapping.Get() == terminality::TextWrap::NoWrap)
			{
				PopFocus(Direction::Down, input.Modifier);
				return true;
			}

			auto lines = TextHelper::GetLines(currentText, arrangedRect_.Width, TextWrapping.Get());
			size_t lineIdx = getLineIndex(lines);

			if (lineIdx + 1 >= lines.size())
			{
				PopFocus(Direction::Down, input.Modifier);
				return true;
			}

			size_t col = cursorPosition_ - lines[lineIdx].StartIndex;
			size_t nextLen = lines[lineIdx + 1].Text.length();

			if (nextLen > 0 && lines[lineIdx + 1].Text.back() == L'\n')
				nextLen--;

			cursorPosition_ = lines[lineIdx + 1].StartIndex + std::min(col, nextLen);
			applyCursorMove();
			return true;
		}

		case InputKey::LEFT:
		{
			if (terminality::hasFlag(input.Modifier, InputModifier::LeftAlt) ||
				terminality::hasFlag(input.Modifier, InputModifier::RightAlt))
			{
				PopFocus(Direction::Left, input.Modifier);
				return true;
			}

			if (cursorPosition_ == 0)
				return true;

			cursorPosition_--;
			applyCursorMove();
			return true;
		}

		case InputKey::RIGHT:
		{
			if (terminality::hasFlag(input.Modifier, InputModifier::LeftAlt) ||
				terminality::hasFlag(input.Modifier, InputModifier::RightAlt))
			{
				PopFocus(Direction::Right, input.Modifier);
				return true;
			}

			if (cursorPosition_ >= currentText.length())
				return true;

			cursorPosition_++;
			applyCursorMove();
			return true;
		}

		case InputKey::BACK:
		{
			if (!canModify)
				return true;

			if (cursorPosition_ == 0)
				return true;

			currentText.erase(--cursorPosition_, 1);
			applyTextChange();
			return true;
		}

		case InputKey::DELETE:
		{
			if (!canModify)
				return true;

			if (cursorPosition_ >= currentText.length())
				return true;

			currentText.erase(cursorPosition_, 1);
			applyTextChange();
			return true;
		}

		case InputKey::HOME:
		{
			if (cursorPosition_ == 0)
				return true;

			cursorPosition_ = 0;
			applyCursorMove();
			return true;
		}

		case InputKey::END:
		{
			if (cursorPosition_ >= currentText.length())
				return true;

			cursorPosition_ = currentText.length();
			applyCursorMove();
			return true;
		}
	}

	return false;
}

bool TextBox::OnKeyUp(InputEvent input)
{
	return false;
}

void TextBox::OnGotFocus()
{
	focused_ = true;
	InvalidateVisual();
}

void TextBox::OnLostFocus()
{
	focused_ = false;
	InvalidateVisual();
}

bool TextBox::MoveFocusNext(Direction direction, InputModifier modifiers)
{
	if (!focusable_)
		return false;

	if (!focused_)
		return true;

	if (modifiers == InputModifier::None && (direction == Direction::Left || direction == Direction::Right))
		return true;

	return false;
}
