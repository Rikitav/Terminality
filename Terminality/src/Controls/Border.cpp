
#include <cstdint>
#include <algorithm>
#include <memory>
#include <cstring>

#include <terminality/Controls/Border.hpp>

using namespace terminality;

namespace
{
	static wchar_t DefaultBorderStyle(const RectanglePos pos)
	{
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

	static wchar_t DefaultBackgroundStyle(const Point& point, const Size& size)
	{
		return L' ';
	}
}

Border::Border()
{
	Style = std::move(DefaultBorderStyle);
	FocusedForegroundColor = Color::CYAN;
	FocusedBackgroundColor = BackgroundColor;
}

Border::Border(std::unique_ptr<ControlBase> content)
{
	Style = std::move(DefaultBorderStyle);
	FocusedForegroundColor = Color::CYAN;
	FocusedBackgroundColor = BackgroundColor;
	Content = std::move(content);
}

void Border::OnContentChanging(const std::unique_ptr<ControlBase>& oldContent)
{
	if (oldContent != nullptr)
		oldContent->SetParent(nullptr);
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
	const Thickness padding = Padding.Get();
	Size desiredSize(thickness, thickness);

	if (Content != nullptr)
	{
		const Size innerSize(
			availableSize.Width >= 0 ? std::max(0, availableSize.Width - thickness - padding.Horizontal()) : -1,
			availableSize.Height >= 0 ? std::max(0, availableSize.Height - thickness - padding.Vertical()) : -1
		);

		const Size childSize = Content.Get()->Measure(innerSize);

		desiredSize.Width += childSize.Width + padding.Horizontal();
		desiredSize.Height += childSize.Height + padding.Vertical();
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

	const Thickness padding = Padding.Get();
	const Rect innerRect(
		contentRect.X + 1 + padding.Left,
		contentRect.Y + 1 + padding.Top,
		std::max(0, contentRect.Width - 2 - padding.Horizontal()),
		std::max(0, contentRect.Height - 2 - padding.Vertical()));

	Content.Get()->Arrange(innerRect);
}

void Border::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	const Color renderColor = focused_ ? FocusedBorderColor : BorderColor;
	const Color background = GetEffectiveBackgroundColor();
	const Color foreground = GetEffectiveForegroundColor();
	const BorderStyle style = Style;

	// Background fill inside the border area.
	if (background != Color::TRANSPARENT)
	{
		context.RenderRectangle(Point(1, 1), Size(rect.Width - 1, rect.Height - 1), foreground, background, DefaultBackgroundStyle);
	}

	for (int32_t x = 0; x < rect.Width; ++x)
	{
		context.SetCell(x, 0,				style(RectanglePos::TopHorizontalLine),    renderColor, background);
		context.SetCell(x, rect.Height - 1, style(RectanglePos::BottomHorizontalLine), renderColor, background);
	}

	for (int32_t y = 0; y < rect.Height; ++y)
	{
		context.SetCell(0, y,				style(RectanglePos::LeftVerticalLine),  renderColor, background);
		context.SetCell(rect.Width - 1, y,	style(RectanglePos::RightVerticalLine), renderColor, background);
	}

	context.SetCell(0, 0,							 style(RectanglePos::LeftTopCorner),	 renderColor, background);
	context.SetCell(0, rect.Height - 1,				 style(RectanglePos::LeftBottomCorner),  renderColor, background);
	context.SetCell(rect.Width - 1, 0,				 style(RectanglePos::RightTopCorner),    renderColor, background);
	context.SetCell(rect.Width - 1, rect.Height - 1, style(RectanglePos::RightBottomCorner), renderColor, background);

	if (HeaderText->size() != 0)
	{
		context.RenderText(Point(2, 0), HeaderText, foreground, background);
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

size_t Border::VisualChildrenCount() const
{
	return Content.Get() ? 1 : 0;
}

VisualTreeNode* Border::GetVisualChild(std::size_t index) const
{
	return Content.Get().get();
}
