module terminality;

import std;
import std.compat;

using namespace terminality;

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

Size TextBox::Measure(const Size& availableSize)
{
	Size desiredSize = ControlBase::Measure(availableSize);
	const int32_t width = std::max<int32_t>(desiredSize.Width, static_cast<int32_t>(text_.length()) + 1);
	
	actualSize_ = Size(width, 1);
	return actualSize_;
}

void TextBox::Render(RenderContext& context)
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

	int32_t viewWidth = rect.Width;
	int32_t offset = 0;
	
	if (static_cast<int32_t>(cursorPosition_) >= viewWidth)
	{
		offset = static_cast<int32_t>(cursorPosition_) - viewWidth + 1;
	}

	std::wstring visibleText = text_.substr(offset, viewWidth);
	context.RenderText(Point(0, 0), visibleText, fore, back, false);

	if (focused_)
	{
		int32_t cursorX = static_cast<int32_t>(cursorPosition_) - offset;
		if (cursorX >= 0 && cursorX < rect.Width)
		{
			wchar_t cursorChar = (cursorPosition_ < text_.length()) ? text_[cursorPosition_] : L' ';
			context.SetCell(cursorX, 0, cursorChar, back, fore);
		}
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

		case InputKey::UP:
		{
			PopFocus(Direction::Up, input.Modifier);
			return;
		}

		case InputKey::DOWN:
		{
			PopFocus(Direction::Down, input.Modifier);
			return;
		}

		case InputKey::LEFT:
		{
			if (terminality::hasFlag(input.Modifier, InputModifier::LeftAlt) || terminality::hasFlag(input.Modifier, InputModifier::RightAlt))
			{
				PopFocus(Direction::Left, input.Modifier);
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
				PopFocus(Direction::Right, input.Modifier);
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

	/*
	switch (direction)
	{
		case NavigationDirection::Right:
		{
			if (hasFlag(modifiers, InputModifier::LeftAlt))
				return false;

			return true;
		}

		case NavigationDirection::Left:
		{
			if (hasFlag(modifiers, InputModifier::LeftAlt))
				return false;

			return true;
		}
	}
	*/

	OnLostFocus();
	return false;
}
