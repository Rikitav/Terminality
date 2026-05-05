module terminality;

import std;
import std.compat;

using namespace terminality;

void ControlBase::SetFocusable(bool value)
{
	if (focusable_ == value)
		return;

	focusable_ = value;
	OnPropertyChanged("Focusable");
}

void ControlBase::SetTabStop(bool value)
{
	if (isTabStop_ == value)
		return;

	isTabStop_ = value;
	OnPropertyChanged("TabStop");
}

void ControlBase::SetTabIndex(int value)
{
	if (tabIndex_ == value)
		return;

	tabIndex_ = value;
	OnPropertyChanged("TabIndex");
}

Size ControlBase::Measure(const Size& availableSize)
{
	int32_t resultWidth = availableSize.Width;
	int32_t resultHeight = availableSize.Height;

	/*
	if (explicitSize_.Width > 0)
		resultWidth = explicitSize_.Width;

	if (explicitSize_.Height > 0)
		resultHeight = explicitSize_.Height;
	*/

	if (MaxSize->Width >= 0)
		resultWidth = resultWidth < 0 ? MaxSize->Width : std::min(resultWidth, MaxSize->Width);

	if (MinSize->Width >= 0)
		resultWidth = std::max(resultWidth, MinSize->Width);
	
	if (MaxSize->Height >= 0)
		resultHeight = resultHeight < 0 ? MaxSize->Height : std::min(resultHeight, MaxSize->Height);
	
	if (MinSize->Height >= 0)
		resultHeight = std::max(resultHeight, MinSize->Height);
	
	// TODO: add margin
	actualSize_ = Size(resultWidth, resultHeight);
	if (actualSize_ == Size::Zero)
		return actualSize_;

	actualSize_ = MeasureOverride(actualSize_);
	return actualSize_;
}

void ControlBase::Arrange(const Rect& finalRect)
{
	Size actualSize = actualSize_;
	int32_t width = actualSize.Width;
	int32_t height = actualSize.Height;

	int32_t x = finalRect.X;
	int32_t y = finalRect.Y;

	switch (HorizontalAlignment)
	{
		case HorizontalAlignment::Left:
		{
			width = width < 0 ? finalRect.Width : std::min(width, finalRect.Width);
			break;
		}

		case HorizontalAlignment::Center:
		{
			width = width < 0 ? finalRect.Width : std::min(width, finalRect.Width);
			x += (finalRect.Width - width) / 2;
			break;
		}

		case HorizontalAlignment::Right:
		{
			width = width < 0 ? finalRect.Width : std::min(width, finalRect.Width);
			x += (finalRect.Width - width);
			break;
		}

		case HorizontalAlignment::Stretch:
		{
			width = std::clamp<uint32_t>(finalRect.Width,
				MinSize->Width == -1 ? 0 : MinSize->Width,
				MaxSize->Width == -1 ? -1 : MaxSize->Width);

			x += (finalRect.Width - width) / 2;
			break;
		}
	}

	switch (VerticalAlignment)
	{
		case VerticalAlignment::Top:
		{
			height = height < 0 ? finalRect.Height : std::min(height, finalRect.Height);
			break;
		}

		case VerticalAlignment::Center:
		{
			height = height < 0
				? finalRect.Height
				: std::min(height, finalRect.Height);
			
			y += (finalRect.Height - height) / 2;
			break;
		}

		case VerticalAlignment::Bottom:
		{
			height = height < 0 ? finalRect.Height : std::min(height, finalRect.Height);
			y += (finalRect.Height - height);
			break;
		}

		case VerticalAlignment::Stretch:
		{
			height = std::clamp<uint32_t>(finalRect.Height,
				MinSize->Height == -1 ? 0 : MinSize->Height,
				MaxSize->Height == -1 ? -1 : MaxSize->Height);

			y += (finalRect.Height - height) / 2;
			break;
		}
	}

	arrangeDirty_ = false;
	arrangedRect_ = Rect(x, y, width, height);
	ArrangeOverride(arrangedRect_);
}

void ControlBase::Render(RenderContext& context)
{
	Rect rect = context.ContextRect();
	context.RenderRectangle(Point::Zero, Size(rect.Width, rect.Height),
		[](const Point& point, const Size& size) { return L' '; });

	visualDirty_ = false;
	RenderOverride(context);
}

void ControlBase::SetParent(VisualTreeNode* parent)
{
	if (parent_ == parent)
		return;

	parent_ = parent;
	OnPropertyChanged("Parent");
}

const std::span<VisualTreeNode*> ControlBase::GetChildren() const
{
	static std::span<VisualTreeNode*> dummy;
	return dummy;
}

void ControlBase::ApplyInvalidation(InvalidationKind invalidation)
{
	switch (invalidation)
	{
		case InvalidationKind::Visual:
		{
			InvalidateVisual();
			break;
		}

		case InvalidationKind::Arrange:
		{
			InvalidateArrange();
			break;
		}

		case InvalidationKind::Measure:
		{
			InvalidateMeasure();
			break;
		}

		default:
		case InvalidationKind::None:
		{
			break;
		}
	}
}

void ControlBase::OnPropertyChanged(const char* propertyName)
{
	return;
}

Size ControlBase::GetActualSize() const
{
	return actualSize_;
}

Rect ControlBase::GetArrangedRect() const
{
	return arrangedRect_;
}
