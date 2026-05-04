module terminality;

import std;
import std.compat;
import :FocusManager;
import :Focus;

using namespace terminality;

namespace
{
	struct LineInfo
	{
		std::wstring Text;
		size_t StartIndex;
	};

	std::vector<LineInfo> GetLines(const std::wstring& text, int32_t availableWidth, TextWrapping wrapping)
	{
		std::vector<LineInfo> lines;
		if (text.empty())
		{
			lines.push_back({ L"", 0 });
			return lines;
		}

		size_t start = 0;
		while (start < text.length())
		{
			size_t end = start;
			int32_t width = 0;

			if (wrapping == TextWrapping::NoWrap || availableWidth <= 0)
			{
				while (end < text.length() && text[end] != L'\n')
					end++;
			}
			else if (wrapping == TextWrapping::Wrap)
			{
				while (end < text.length() && text[end] != L'\n' && width < availableWidth)
				{
					end++;
					width++;
				}
			}
			else // WrapWholeWords
			{
				size_t lastSpace = std::wstring::npos;
				while (end < text.length() && text[end] != L'\n' && width < availableWidth)
				{
					if (text[end] == L' ')
						lastSpace = end;
					end++;
					width++;
				}

				if (end < text.length() && text[end] != L'\n' && text[end] != L' ' && lastSpace != std::wstring::npos && lastSpace > start)
				{
					end = lastSpace; // Break at last space
				}
			}

			lines.push_back({ text.substr(start, end - start), start });
			start = end;

			if (start < text.length() && text[start] == L'\n')
				start++;
			else if (wrapping == TextWrapping::WrapWholeWords && start < text.length() && text[start] == L' ')
				start++; // Skip space at beginning of next line
		}

		if (!text.empty() && text.back() == L'\n')
		{
			lines.push_back({ L"", text.length() });
		}

		return lines;
	}
}

TextBox::TextBox()
{
	isTabStop_ = true;
}

void TextBox::SetText(std::wstring text)
{
	if (text_ == text)
		return;

	text_ = std::move(text);
	cursorPosition_ = std::min(cursorPosition_, text_.length());
	InvalidateMeasure();
	InvalidateVisual();
	TextChanged.Emit();
}

std::wstring TextBox::GetText() const
{
	return text_;
}

TextWrapping TextBox::GetTextWrapping() const
{
	return textWrapping_;
}

void TextBox::SetTextWrapping(TextWrapping textWrapping)
{
	if (textWrapping_ == textWrapping)
		return;

	textWrapping_ = textWrapping;
	InvalidateMeasure();
	InvalidateVisual();
}

TextAlignment TextBox::GetTextAlignment() const
{
	return textAlignment_;
}

void TextBox::SetTextAlignment(TextAlignment textAlignment)
{
	if (textAlignment_ == textAlignment)
		return;

	textAlignment_ = textAlignment;
	InvalidateVisual();
}

bool TextBox::GetAcceptsReturn() const
{
	return acceptsReturn_;
}

void TextBox::SetAcceptsReturn(bool acceptsReturn)
{
	if (acceptsReturn_ == acceptsReturn)
		return;

	acceptsReturn_ = acceptsReturn;
	InvalidateMeasure();
	InvalidateVisual();
}

Size TextBox::MeasureOverride(const Size& availableSize)
{
	std::vector<LineInfo> lines = GetLines(text_, availableSize.Width, textWrapping_);

	int32_t maxWidth = availableSize.Width;
	for (const auto& line : lines)
		maxWidth = std::clamp(static_cast<int32_t>(line.Text.length()), 0, availableSize.Width);

	int32_t width = std::clamp(maxWidth + 1, 0, availableSize.Width);
	int32_t height = availableSize.Height;

	if (textWrapping_ != TextWrapping::NoWrap && availableSize.Width > 0)
		width = std::min(width, availableSize.Width);

	return Size(width, height);
}

void TextBox::ArrangeOverride(const Rect& contentRect)
{
	return;
}

void TextBox::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();

	Color fore = focused_ ? Color::BLACK : Color::WHITE;
	Color back = focused_ ? Color::WHITE : Color::DARK_GRAY;

	for (int32_t y = 0; y < rect.Height; ++y)
	{
		for (int32_t x = 0; x < rect.Width; ++x)
		{
			context.SetCell(x, y, L' ', fore, back);
		}
	}

	std::vector<LineInfo> lines = GetLines(text_, rect.Width, textWrapping_);

	int32_t cursorY = 0;
	int32_t cursorX = 0;
	bool cursorFound = false;

	// First find the cursor
	for (size_t y = 0; y < lines.size(); ++y)
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

	if (textWrapping_ == TextWrapping::NoWrap)
	{
		if (cursorX >= viewWidth)
		{
			offset = cursorX - viewWidth + 1;
		}
	}

	// Adjust cursorX based on scroll offset
	cursorX -= offset;

	for (int32_t y = 0; y < std::min(rect.Height, static_cast<int32_t>(lines.size())); ++y)
	{
		const auto& line = lines[y];
		int32_t xOffset = 0;

		std::wstring visibleText = line.Text;
		if (offset > 0 && offset < visibleText.length())
			visibleText = visibleText.substr(offset);
		else if (offset >= visibleText.length())
			visibleText = L"";

		int32_t textLen = static_cast<int32_t>(visibleText.length());
		if (textAlignment_ == TextAlignment::Center)
		{
			xOffset = std::max(0, (rect.Width - textLen) / 2);
		}
		else if (textAlignment_ == TextAlignment::Right)
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

	if (focused_ && cursorY < rect.Height && cursorX < rect.Width && cursorX >= 0)
	{
		wchar_t cursorChar = (cursorPosition_ < text_.length() && text_[cursorPosition_] != L'\n') ? text_[cursorPosition_] : L' ';
		context.SetCell(cursorX, cursorY, cursorChar, back, fore);
	}

	visualDirty_ = false;
}

void TextBox::OnKeyDown(InputEvent input)
{
	bool handled = false;
	bool textChanged = false;

	switch (input.Key)
	{
		default:
		{
			if (input.Char != L'\0' && input.Char >= 32)
			{
				text_.insert(cursorPosition_, 1, input.Char);
				cursorPosition_++;
				handled = true;
				textChanged = true;
			}

			break;
		}

		case InputKey::SPACE:
		{
			text_.insert(cursorPosition_, 1, L' ');
			cursorPosition_++;
			handled = true;
			textChanged = true;
			break;
		}

		case InputKey::RETURN:
		{
			if (acceptsReturn_)
			{
				text_.insert(cursorPosition_, 1, L'\n');
				cursorPosition_++;
				handled = true;
				textChanged = true;
			}
			break;
		}

		case InputKey::UP:
		{
			if (acceptsReturn_ || textWrapping_ != TextWrapping::NoWrap)
			{
				auto lines = GetLines(text_, arrangedRect_.Width, textWrapping_);
				for (size_t i = 0; i < lines.size(); ++i)
				{
					if (cursorPosition_ >= lines[i].StartIndex && (i + 1 == lines.size() || cursorPosition_ < lines[i + 1].StartIndex))
					{
						if (i > 0)
						{
							size_t col = cursorPosition_ - lines[i].StartIndex;
							size_t prevLen = lines[i - 1].Text.length();
							if (!lines[i - 1].Text.empty() && lines[i - 1].Text.back() == L'\n')
								prevLen--;
							cursorPosition_ = lines[i - 1].StartIndex + std::min(col, prevLen);
							handled = true;
						}
						else
						{
							FocusManager::Current().MoveNext(Direction::Up, input.Modifier);
							return;
						}
						break;
					}
				}
			}
			else
			{
				FocusManager::Current().MoveNext(Direction::Up, input.Modifier);
			}
			return;
		}

		case InputKey::DOWN:
		{
			if (acceptsReturn_ || textWrapping_ != TextWrapping::NoWrap)
			{
				auto lines = GetLines(text_, arrangedRect_.Width, textWrapping_);
				for (size_t i = 0; i < lines.size(); ++i)
				{
					if (cursorPosition_ >= lines[i].StartIndex && (i + 1 == lines.size() || cursorPosition_ < lines[i + 1].StartIndex))
					{
						if (i + 1 < lines.size())
						{
							size_t col = cursorPosition_ - lines[i].StartIndex;
							size_t nextLen = lines[i + 1].Text.length();
							if (!lines[i + 1].Text.empty() && lines[i + 1].Text.back() == L'\n')
								nextLen--;

							cursorPosition_ = lines[i + 1].StartIndex + std::min(col, nextLen);
							handled = true;
						}
						else
						{
							FocusManager::Current().MoveNext(Direction::Down, input.Modifier);
							return;
						}
						break;
					}
				}
			}
			else
			{
				FocusManager::Current().MoveNext(Direction::Down, input.Modifier);
			}
			return;
		}

		case InputKey::LEFT:
		{
			if (terminality::hasFlag(input.Modifier, InputModifier::LeftAlt) || terminality::hasFlag(input.Modifier, InputModifier::RightAlt))
			{
				FocusManager::Current().MoveNext(Direction::Left, input.Modifier);
				return;
			}

			if (cursorPosition_ > 0)
			{
				cursorPosition_--;
				handled = true;
			}
			/*
			else
			{
				PopFocus(Direction::Right, input.Modifier);
				return;
			}
			*/

			break;
		}

		case InputKey::RIGHT:
		{
			if (terminality::hasFlag(input.Modifier, InputModifier::LeftAlt) || terminality::hasFlag(input.Modifier, InputModifier::RightAlt))
			{
				FocusManager::Current().MoveNext(Direction::Right, input.Modifier);
				return;
			}

			if (cursorPosition_ < text_.length())
			{
				cursorPosition_++;
				handled = true;
			}
			/*
			else
			{
				PopFocus(Direction::Right, input.Modifier);
				return;
			}
			*/

			break;
		}

		case InputKey::BACK:
		{
			if (cursorPosition_ > 0)
			{
				text_.erase(cursorPosition_ - 1, 1);
				cursorPosition_--;
				handled = true;
				textChanged = true;
			}

			break;
		}

		case InputKey::DELETE:
		{
			if (cursorPosition_ < text_.length())
			{
				text_.erase(cursorPosition_, 1);
				handled = true;
				textChanged = true;
			}

			break;
		}

		case InputKey::HOME:
		{
			if (cursorPosition_ > 0)
			{
				cursorPosition_ = 0;
				handled = true;
			}

			break;
		}

		case InputKey::END:
		{
			if (cursorPosition_ < text_.length())
			{
				cursorPosition_ = text_.length();
				handled = true;
			}

			break;
		}
	}

	if (handled)
	{
		InvalidateMeasure();
		InvalidateVisual();

		if (textChanged)
		{
			TextChanged.Emit();
		}
	}
}

void TextBox::OnKeyUp(InputEvent input)
{
	return;
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
	{
		return false;
	}

	if (!focused_)
	{
		OnGotFocus();
		return true;
	}

	if (modifiers == InputModifier::None && (direction == Direction::Left || direction == Direction::Right))
	{
		return true;
	}

	OnLostFocus();
	return false;
}
