module terminality;

import std;
import std.compat;

using namespace terminality;

void Button::Click()
{
	Clicked.Emit();
	InvalidateVisual();
}

void Button::OnPropertyChanged(const char* propertyName)
{
	ControlBase::OnPropertyChanged(propertyName);
}

bool Button::OnKeyDown(InputEvent input)
{
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
	int32_t contentWidth = static_cast<int32_t>(Text->size()) + 6;
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
	std::wstring line = L"[  " + Text.Get() + L"  ]";

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
