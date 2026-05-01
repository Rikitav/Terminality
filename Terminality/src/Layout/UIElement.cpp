module terminality;

import std;

using namespace terminality;

Size UIElement::GetSize() const
{
	return explicitSize_;
}

Size UIElement::GetMinSize() const
{
	return minSize_;
}

Size UIElement::GetMaxSize() const
{
	return maxSize_;
}

Size UIElement::GetActualSize() const
{
	return actualSize_;
}

Rect UIElement::GetArrangedRect() const
{
	return arrangedRect_;
}

Thickness UIElement::GetMargin() const
{
	return margin_;
}

HorizontalAlignment UIElement::GetHorizontalAlignment() const
{
	return horizontalAlignment_;
}

VerticalAlignment UIElement::GetVerticalAlignment() const
{
	return verticalAlignment_;
}

/*
Size UIElement::MeasureContent(const Size& availableSize)
{
	const Size innerAvailable(
		std::max(0, availableSize.Width - margin_.Horizontal()),
		std::max(0, availableSize.Height - margin_.Vertical()));

	lastMeasuredContentSize_ = MeasureContent(innerAvailable);
	const Size withLayout = CoerceWithLayoutRules(lastMeasuredContentSize_);
	return Size(withLayout.Width + margin_.Horizontal(), withLayout.Height + margin_.Vertical());
}
*/

/*
void UIElement::ArrangeContent(const Rect& finalRect)
{
	const Rect innerRect(
		finalRect.X + margin_.Left,
		finalRect.Y + margin_.Top,
		std::max(0, finalRect.Width - margin_.Horizontal()),
		std::max(0, finalRect.Height - margin_.Vertical()));

	int32_t contentWidth = std::max(minSize_.Width, lastMeasuredContentSize_.Width);
	int32_t contentHeight = std::max(minSize_.Height, lastMeasuredContentSize_.Height);

	if (explicitWidth_ >= 0)
		contentWidth = explicitWidth_;

	if (explicitHeight_ >= 0)
		contentHeight = explicitHeight_;

	contentWidth = std::min(contentWidth, innerRect.Width);
	contentHeight = std::min(contentHeight, innerRect.Height);

	int32_t contentX = innerRect.X;
	int32_t contentY = innerRect.Y;

	switch (horizontalAlignment_)
	{
		case HorizontalAlignment::Center:
		{
			contentX = innerRect.X + std::max(0, (innerRect.Width - contentWidth) / 2);
			break;
		}

		case HorizontalAlignment::Right:
		{
			contentX = innerRect.X + std::max(0, innerRect.Width - contentWidth);
			break;
		}

		case HorizontalAlignment::Stretch:
		{
			contentWidth = innerRect.Width;
			break;
		}
	}

	switch (verticalAlignment_)
	{
		case VerticalAlignment::Center:
		{
			contentY = innerRect.Y + std::max(0, (innerRect.Height - contentHeight) / 2);
			break;
		}

		case VerticalAlignment::Bottom:
		{
			contentY = innerRect.Y + std::max(0, innerRect.Height - contentHeight);
			break;
		}

		case VerticalAlignment::Stretch:
		{
			contentHeight = innerRect.Height;
			break;
		}
	}

	Rect contentRect = Rect(contentX, contentY, contentWidth, contentHeight);
	ArrangeContent(contentRect);
}
*/

/*
Size UIElement::CoerceWithLayoutRules(const Size& contentSize) const
{
	Size coerced(contentSize);
	coerced.Width = std::max(coerced.Width, minSize_.Width);
	coerced.Height = std::max(coerced.Height, minSize_.Height);
	
	if (explicitWidth_ >= 0)
		coerced.Width = explicitWidth_;

	if (explicitHeight_ >= 0)
		coerced.Height = explicitHeight_;

	return coerced;
}
*/
