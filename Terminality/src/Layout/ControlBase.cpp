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

	if (maxSize_.Width >= 0)
		resultWidth = std::min(resultWidth, maxSize_.Width);

	if (minSize_.Width >= 0)
		resultWidth = std::max(resultWidth, minSize_.Width);
	
	if (maxSize_.Height >= 0)
		resultHeight = std::min(resultHeight, maxSize_.Height);
	
	if (minSize_.Height >= 0)
		resultHeight = std::max(resultHeight, minSize_.Height);
	
	actualSize_ = Size(resultWidth, resultHeight);
	return actualSize_;
}

void ControlBase::Arrange(const Rect& finalRect)
{
	int32_t width = actualSize_.Width;
	int32_t height = actualSize_.Height;

	int32_t x = finalRect.X;
	int32_t y = finalRect.Y;

	switch (horizontalAlignment_)
	{
		case HorizontalAlignment::Left:
		{
			width = std::min(width, finalRect.Width);
			break;
		}

		case HorizontalAlignment::Center:
		{
			width = std::min(width, finalRect.Width);
			x += (finalRect.Width - width) / 2;
			break;
		}

		case HorizontalAlignment::Right:
		{
			width = std::min(width, finalRect.Width);
			x += (finalRect.Width - width);
			break;
		}

		case HorizontalAlignment::Stretch:
		{
			width = finalRect.Width;
			break;
		}
	}

	switch (verticalAlignment_)
	{
		case VerticalAlignment::Top:
		{
			height = std::min(height, finalRect.Height);
			break;
		}

		case VerticalAlignment::Center:
		{
			height = std::min(height, finalRect.Height);
			y += (finalRect.Height - height) / 2;
			break;
		}

		case VerticalAlignment::Bottom:
		{
			height = std::min(height, finalRect.Height);
			y += (finalRect.Height - height);
			break;
		}

		case VerticalAlignment::Stretch:
		{
			height = finalRect.Height;
			break;
		}
	}

	arrangedRect_ = Rect(x, y, width, height);
	arrangeDirty_ = false;
}

void ControlBase::Render(RenderContext& context)
{
	Rect rect = context.ContextRect();
	context.RenderRectangle(
		Point(0, 0),
		Size(rect.Width, rect.Height),
		[](const Point& point, const Size& size) { return L' '; });

	visualDirty_ = false;

	/*
	Rect rect = context.ContextRect();
	context.RenderRectangle(
		Point(rect.X, rect.Y),
		Size(rect.Width, rect.Height),
		[](const Point& point, const Size& size) { return L' '; });

	visualDirty_ = false;
	*/
}

const std::span<VisualTreeNode*> ControlBase::GetChildren() const
{
	static std::span<VisualTreeNode*> dummy;
	return dummy;
}