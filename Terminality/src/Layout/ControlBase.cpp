module terminality;

import std;
import std.compat;

using namespace terminality;

void ControlBase::SetParent(VisualTreeNode* parent)
{
	if (parent_ == parent)
		return;

	parent_ = parent;
	OnPropertyChanged("Parent");
}

void ControlBase::SetSize(Size value)
{
	if (explicitSize_ == value)
		return;

	explicitSize_ = value;
	OnPropertyChanged("Size");
}

void ControlBase::SetMinSize(Size value)
{
	if (minSize_ == value)
		return;

	minSize_ = value;
	OnPropertyChanged("MinSize");
}

void ControlBase::SetMaxSize(Size value)
{
	if (maxSize_ == value)
		return;

	maxSize_ = value;
	OnPropertyChanged("MaxSize");
}

void ControlBase::SetMargin(Thickness value)
{
	if (margin_ == value)
		return;

	margin_ = value;
	OnPropertyChanged("Margin");
}

void ControlBase::SetHorizontalAlignment(HorizontalAlignment value)
{
	if (horizontalAlignment_ == value)
		return;

	horizontalAlignment_ = value;
	InvalidateArrange();
	OnPropertyChanged("HorizontalAlignment");
}

void ControlBase::SetVerticalAlignment(VerticalAlignment value)
{
	if (verticalAlignment_ == value)
		return;

	verticalAlignment_ = value;
	InvalidateArrange();
	OnPropertyChanged("VerticalAlignment");
}

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

	if (explicitSize_.Width > 0)
		resultWidth = explicitSize_.Width;

	if (explicitSize_.Height > 0)
		resultHeight = explicitSize_.Height;

	if (maxSize_.Width >= 0)
		resultWidth = resultWidth < 0 ? maxSize_.Width : std::min(resultWidth, maxSize_.Width);

	if (minSize_.Width >= 0)
		resultWidth = std::max(resultWidth, minSize_.Width);
	
	if (maxSize_.Height >= 0)
		resultHeight = resultHeight < 0 ? maxSize_.Height : std::min(resultHeight, maxSize_.Height);
	
	if (minSize_.Height >= 0)
		resultHeight = std::max(resultHeight, minSize_.Height);
	
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

	switch (horizontalAlignment_)
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
				minSize_.Width == -1 ? 0 : minSize_.Width,
				maxSize_.Width == -1 ? -1 : maxSize_.Width);

			x += (finalRect.Width - width) / 2;
			break;
		}
	}

	switch (verticalAlignment_)
	{
		case VerticalAlignment::Top:
		{
			height = height < 0 ? finalRect.Height : std::min(height, finalRect.Height);
			break;
		}

		case VerticalAlignment::Center:
		{
			height = height < 0 ? finalRect.Height : std::min(height, finalRect.Height);
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
				minSize_.Height == -1 ? 0 : minSize_.Height,
				maxSize_.Height == -1 ? -1 : maxSize_.Height);

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
	context.RenderRectangle(
		Point(0, 0),
		Size(rect.Width, rect.Height),
		[](const Point& point, const Size& size) { return L' '; });

	visualDirty_ = false;
	RenderOverride(context);
}

const std::span<VisualTreeNode*> ControlBase::GetChildren() const
{
	static std::span<VisualTreeNode*> dummy;
	return dummy;
}