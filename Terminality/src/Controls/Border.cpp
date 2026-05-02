module terminality;

import std;
import std.compat;

using namespace terminality;

void Border::SetContent(std::unique_ptr<ControlBase> content)
{
	if (content)
		content->SetParent(this);

	content_ = std::move(content);
	InvalidateMeasure();
	InvalidateVisual();
}

ControlBase* Border::GetContent() const
{
	return content_.get();
}

Color Border::GetBorderColor() const
{
	return borderColor_;
}

/*
Thickness Border::GetBorderThickness() const
{
	return borderThickness_;
}
*/

void Border::SetBorderColor(Color color)
{
	if (borderColor_ == color)
		return;

	borderColor_ = color;
	InvalidateVisual();
	OnPropertyChanged("BorderColor");
}

/*
void Border::SetBorderThickness(Thickness thickness)
{
	borderThickness_ = thickness;
	InvalidateMeasure();
	InvalidateVisual();
	OnPropertyChanged("BorderThickness");
}
*/

bool Border::MoveFocusNext(NavigationDirection direction)
{
	OnLostFocus();
	return false;
}

void Border::OnGotFocus()
{
	if (content_ == nullptr)
	{
		InvalidateVisual();
		return;
	}

	VisualTreeNode* focusedControl = content_.get();
	FocusManager::Current().SetFocused(focusedControl);
	InvalidateVisual();
}

void Border::OnLostFocus()
{
	focused_ = false;
	InvalidateVisual();
}

Size Border::Measure(const Size& availableSize)
{
	Size desiredSize = ControlBase::Measure(availableSize);
	if (content_ == nullptr)
		return Size(2, 2);

	const Size innerSize(std::max(0, desiredSize.Width - 2), std::max(0, desiredSize.Height - 2));
	const Size childSize = content_->Measure(innerSize);
	
	actualSize_ = Size(std::min(innerSize.Width, childSize.Width + 2), std::min(innerSize.Height, childSize.Height + 2));
	return actualSize_;
}

void Border::Arrange(const Rect& contentRect)
{
	ControlBase::Arrange(contentRect);
	Rect arrangedRect = GetArrangedRect();

	if (content_ != nullptr)
	{
		const Rect innerRect(1, 1,
			std::max(0, arrangedRect.Width - 2),
			std::max(0, arrangedRect.Height - 2));

		content_->Arrange(innerRect);
	}

	arrangedRect_ = arrangedRect;
	arrangeDirty_ = false;
}

void Border::Render(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	
	for (int32_t x = 0; x < rect.Width; ++x)
	{
		context.SetCell(x, 0,				L'\x2500', borderColor_, Color::BLACK);
		context.SetCell(x, rect.Height - 1,	L'\x2500', borderColor_, Color::BLACK);
	}

	for (int32_t y = 0; y < rect.Height; ++y)
	{
		context.SetCell(0, y,				L'\x2502', borderColor_, Color::BLACK);
		context.SetCell(rect.Width - 1, y,	L'\x2502', borderColor_, Color::BLACK);
	}

	context.SetCell(0, 0,							 L'\x256D', borderColor_, Color::BLACK);
	context.SetCell(rect.Width - 1, 0,				 L'\x256E', borderColor_, Color::BLACK);
	context.SetCell(0, rect.Height - 1,				 L'\x2570', borderColor_, Color::BLACK);
	context.SetCell(rect.Width - 1, rect.Height - 1, L'\x256F', borderColor_, Color::BLACK);

	if (rect.Width > 2 || rect.Height > 2)
	{
		Point point(1, 1);
		Size size(rect.Width - 2, rect.Height - 2);
		context.RenderRectangle(point, size, [](const Point& point, const Size& size) { return L' '; });

		if (content_ != nullptr)
		{
			Rect childRect = content_->GetArrangedRect();
			RenderContext childContext = context.CreateInner(childRect);
			content_->Render(childContext);
		}
	}

	visualDirty_ = false;
}