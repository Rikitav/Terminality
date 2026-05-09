module terminality;

import std;
import std.compat;

using namespace terminality;

static wchar_t DefaultBorderStyle(const RectanglePos pos)
{
	/*
	
	for (int32_t x = 0; x < rect.Width; ++x)
	{
		context.SetCell(x, 0,				L'\x2500', renderColor, BackgroundColor);
		context.SetCell(x, rect.Height - 1,	L'\x2500', renderColor, BackgroundColor);
	}

	for (int32_t y = 0; y < rect.Height; ++y)
	{
		context.SetCell(0, y,				L'\x2502', renderColor, BackgroundColor);
		context.SetCell(rect.Width - 1, y,	L'\x2502', renderColor, BackgroundColor);
	}

	context.SetCell(0, 0,							 L'\x256D', renderColor, BackgroundColor);
	context.SetCell(rect.Width - 1, 0,				 L'\x256E', renderColor, BackgroundColor);
	context.SetCell(0, rect.Height - 1,				 L'\x2570', renderColor, BackgroundColor);
	context.SetCell(rect.Width - 1, rect.Height - 1, L'\x256F', renderColor, BackgroundColor);

	*/

	switch (pos)
	{
		case RectanglePos::TopHorizontalLine:
		case RectanglePos::BottomHorizontalLine:
			return L'\x2500';

		case RectanglePos::LeftVerticalLine:
		case RectanglePos::RightVerticalLine:
			return L'\x2502';

		case RectanglePos::LeftTopCorner:
			return L'\x256D';

		case RectanglePos::LeftBottomCorner:
			return L'\x2570';

		case RectanglePos::RightTopCorner:
			return L'\x256E';

		case RectanglePos::RightBottomCorner:
			return L'\x256F';

		default:
			return L'#';
	}
}

Border::Border()
{
	Style = std::move(DefaultBorderStyle);
	FocusedForegroundColor = Color::CYAN;
	FocusedBackgroundColor = Color::CYAN;
}

Border::Border(std::unique_ptr<ControlBase> content)
{
	Style = std::move(DefaultBorderStyle);
	FocusedForegroundColor = Color::CYAN;
	FocusedBackgroundColor = Color::CYAN;
	Content = std::move(content);
}

void Border::OnPropertyChanged(const char* propertyName)
{
	if (std::strcmp(propertyName, "Content") == 0)
	{
		if (Content.Get() != nullptr)
			Content.Get()->SetParent(this, layer_);

		InvalidateMeasure();
		InvalidateVisual();
		return;
	}

	ControlBase::OnPropertyChanged(propertyName);
}

bool Border::MoveFocusNext(Direction direction, InputModifier modifiers)
{
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
	Size desiredSize(thickness, thickness);

	if (Content != nullptr)
	{
		const Size innerSize(
			availableSize.Width >= 0 ? std::max(0, availableSize.Width - thickness) : -1,
			availableSize.Height >= 0 ? std::max(0, availableSize.Height - thickness) : -1
		);

		const Size childSize = Content.Get()->Measure(innerSize);

		desiredSize.Width += childSize.Width;
		desiredSize.Height += childSize.Height;
	}

	if (availableSize.Width >= 0)
		desiredSize.Width = std::min(desiredSize.Width, availableSize.Width);

	if (availableSize.Height >= 0)
		desiredSize.Height = std::min(desiredSize.Height, availableSize.Height);

	return desiredSize;
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
	const Size size = rect.Size();
	const Color renderColor = focused_ ? FocusedBorderColor : BorderColor;
	const BorderStyle style = Style;

	for (int32_t x = 0; x < rect.Width; ++x)
	{
		context.SetCell(x, 0,				style(RectanglePos::TopHorizontalLine),    renderColor, BackgroundColor);
		context.SetCell(x, rect.Height - 1, style(RectanglePos::BottomHorizontalLine), renderColor, BackgroundColor);
	}

	for (int32_t y = 0; y < rect.Height; ++y)
	{
		context.SetCell(0, y,				style(RectanglePos::LeftVerticalLine),  renderColor, BackgroundColor);
		context.SetCell(rect.Width - 1, y,	style(RectanglePos::RightVerticalLine), renderColor, BackgroundColor);
	}

	context.SetCell(0, 0,							 style(RectanglePos::LeftTopCorner),	 renderColor, BackgroundColor);
	context.SetCell(0, rect.Height - 1,				 style(RectanglePos::LeftBottomCorner),  renderColor, BackgroundColor);
	context.SetCell(rect.Width - 1, 0,				 style(RectanglePos::RightTopCorner),    renderColor, BackgroundColor);
	context.SetCell(rect.Width - 1, rect.Height - 1, style(RectanglePos::RightBottomCorner), renderColor, BackgroundColor);

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
