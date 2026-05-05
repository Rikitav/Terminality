module terminality;

import std;
import std.compat;

using namespace terminality;

void CheckBox::Toggle(std::optional<bool> value)
{
	isChecked_ = value;
	Toggled.Emit(isChecked_);
	InvalidateVisual();

	if (isChecked_.has_value())
	{
		if (isChecked_.value())
			Checked.Emit();
		else
			Unchecked.Emit();
	}
}

void CheckBox::OnPropertyChanged(const char* propertyName)
{
	ControlBase::OnPropertyChanged(propertyName);
}

void CheckBox::OnKeyDown(InputEvent input)
{
	switch (input.Key)
	{
		case InputKey::RETURN:
		case InputKey::SPACE:
		{
			isPressed_ = true;
			InvalidateVisual();
			break;
		}

		default:
		{
			VisualTreeNode::OnKeyDown(input);
			break;
		}
	}
}

void CheckBox::OnKeyUp(InputEvent input)
{
	switch (input.Key)
	{
		case InputKey::RETURN:
		case InputKey::SPACE:
		{
			isPressed_ = false;
			if (!isChecked_.has_value())
			{
				isChecked_ = true;
			}
			else if (isChecked_.value())
			{
				isChecked_ = false;
			}
			else
			{
				isChecked_ = true;
			}

			InvalidateVisual();
			break;
		}

		default:
		{
			VisualTreeNode::OnKeyUp(input);
			break;
		}
	}
}

void CheckBox::OnLostFocus()
{
	focused_ = false;
	isPressed_ = false;
	InvalidateVisual();
}

Size CheckBox::MeasureOverride(const Size& availableSize)
{
	int32_t contentWidth = static_cast<int32_t>(Text->size()) + 7 + 3;
	int32_t width = availableSize.Width >= 0 ? std::min(availableSize.Width, contentWidth) : contentWidth;
	int32_t height = availableSize.Height >= 0 ? std::min(availableSize.Height, 1) : 1;
	return Size(width, height);
}

void CheckBox::ArrangeOverride(const Rect& contentRect)
{
	// bleh U_U
	return;
}

void CheckBox::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	std::wstring line = L"";

	if (!isChecked_.has_value())
	{
		line += L"[?]";
	}
	else if (isChecked_.value())
	{
		line += L"[X]";
	}
	else
	{
		line += L"[ ]";
	}

	line += L" [  " + Text.Get() + L"  ]";
	Color fore = ForegroundColor;
	Color back = BackgroundColor;

	if (focused_)
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

	context.RenderText(Point::Zero, line, fore, back, false);
}
