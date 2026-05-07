module terminality;

import std;
import std.compat;

using namespace terminality;

Border::Border()
{
	FocusedForegroundColor = Color::CYAN;
	FocusedBackgroundColor = Color::CYAN;
}

Border::Border(std::unique_ptr<ControlBase> content)
{
	FocusedForegroundColor = Color::CYAN;
	FocusedBackgroundColor = Color::CYAN;
	Content = std::move(content);
}

void Border::OnPropertyChanged(const char* propertyName)
{
	if (std::strcmp(propertyName, "Content") == 0)
	{
		if (Content.Get() != nullptr)
			Content.Get()->SetParent(this);

		InvalidateMeasure();
		InvalidateVisual();
		return;
	}

	ControlBase::OnPropertyChanged(propertyName);
}

bool Border::MoveFocusNext(Direction direction, InputModifier modifiers)
{
	OnLostFocus();
	return false;
}

void Border::OnGotFocus()
{
	if (Content == nullptr)
	{
		InvalidateVisual();
		return;
	}

	focused_ = true;
	VisualTreeNode* focusedControl = Content->get();
	PushFocus(focusedControl);
	InvalidateVisual();
}

void Border::OnLostFocus()
{
	focused_ = false;
	InvalidateVisual();
}

Size Border::MeasureOverride(const Size& availableSize)
{
	const int thickness = 2;
	Size result(
		availableSize.Width >= 0 ? availableSize.Width : thickness,
		availableSize.Height >= 0 ? availableSize.Height : thickness);

	if (Content != nullptr)
	{
		const Size innerSize(
			availableSize.Width >= 0 ? std::max(0, availableSize.Width - thickness) : -1,
			availableSize.Height >= 0 ? std::max(0, availableSize.Height - thickness) : -1
		);

		const Size childSize = Content.Get()->Measure(innerSize);
		if (availableSize.Width < 0)
			result.Width = std::max(thickness, childSize.Width + thickness);

		if (availableSize.Height < 0)
			result.Height = std::max(thickness, childSize.Height + thickness);
	}

	return result;
}

void Border::ArrangeOverride(const Rect& contentRect)
{
	if (Content == nullptr)
		return;

	Rect arrangedRect = GetArrangedRect();
	const Rect innerRect(
		contentRect.X + 1,
		contentRect.Y + 1,
		std::max(0, arrangedRect.Width - 2),
		std::max(0, arrangedRect.Height - 2));

	Content.Get()->Arrange(innerRect);
}

void Border::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	Color renderColor = focused_ ? FocusedForegroundColor : BorderColor;
	
	for (int32_t x = 0; x < rect.Width; ++x)
	{
		context.SetCell(x, 0,				L'\x2500', renderColor, Color::BLACK);
		context.SetCell(x, rect.Height - 1,	L'\x2500', renderColor, Color::BLACK);
	}

	for (int32_t y = 0; y < rect.Height; ++y)
	{
		context.SetCell(0, y,				L'\x2502', renderColor, Color::BLACK);
		context.SetCell(rect.Width - 1, y,	L'\x2502', renderColor, Color::BLACK);
	}

	context.SetCell(0, 0,							 L'\x256D', renderColor, Color::BLACK);
	context.SetCell(rect.Width - 1, 0,				 L'\x256E', renderColor, Color::BLACK);
	context.SetCell(0, rect.Height - 1,				 L'\x2570', renderColor, Color::BLACK);
	context.SetCell(rect.Width - 1, rect.Height - 1, L'\x256F', renderColor, Color::BLACK);

	if (HeaderText->size() != 0)
	{
		context.RenderText(Point(2, 0), HeaderText, FocusedForegroundColor, BackgroundColor);
	}

	if (rect.Width > 2 || rect.Height > 2)
	{
		if (Content != nullptr)
		{
			Rect childRect = Content.Get()->GetArrangedRect();
			RenderContext childContext = context.CreateInner(childRect);
			Content.Get()->Render(childContext);
		}
	}
}
