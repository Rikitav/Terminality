module terminality;

import std;
import std.compat;

using namespace terminality;

void StackPanel::AddChild(std::unique_ptr<ControlBase> child)
{
	if (!child)
		return;

	child->SetParent(this);
	if (!child->IsAttached())
		child->OnAttachedToTree();

	contents_.push_back(std::move(child));
	InvalidateMeasure();
}

std::unique_ptr<ControlBase> StackPanel::RemoveChild(ControlPredicate predicate)
{
	const auto found = std::find_if(contents_.begin(), contents_.end(),
		[&](const std::unique_ptr<ControlBase>& control) { return predicate(control.get()); });

	if (found == contents_.end())
		return nullptr;

	std::unique_ptr<ControlBase> removed = std::move(*found);
	contents_.erase(found);

	if (removed)
	{
		removed->SetParent(nullptr);
		removed->OnDettachedFromTree();
	}

	if (focusedIndex_ >= contents_.size())
		focusedIndex_ = contents_.empty() ? 0 : contents_.size() - 1;

	InvalidateMeasure();
	return removed;
}

void StackPanel::OnGotFocus()
{
	if (contents_.empty())
	{
		InvalidateVisual();
		return;
	}

	if (focusedIndex_ > contents_.size() - 1)
		focusedIndex_ = contents_.size() - 1;

	if (focusedIndex_ < 0)
		focusedIndex_ = 0;

	VisualTreeNode* focusedControl = contents_[focusedIndex_].get();
	PushFocus(focusedControl);
	InvalidateVisual();
}

void StackPanel::OnLostFocus()
{
	focused_ = false;
	InvalidateVisual();
}

bool StackPanel::MoveFocusNext(Direction direction, InputModifier modifiers)
{
	if (contents_.empty())
		return false;

	switch (direction)
	{
		case Direction::Left:
		case Direction::Right:
			return false;

		case Direction::Up:
		case Direction::Previous:
		{
			if (focusedIndex_ == 0)
				focusedIndex_ = contents_.size() - 1;
			else
				focusedIndex_ -= 1;

			for (size_t i = focusedIndex_; i < contents_.size(); i++)
			{
				ControlBase* control = contents_[i].get();
				if (control->IsFocusable() && control->IsTabStop())
				{
					focusedIndex_ = i;
					PushFocus(control);
					return true;
				}
			}

			break;
		}

		case Direction::Down:
		case Direction::Next:
		{
			if (contents_.size() - 1 == focusedIndex_)
				focusedIndex_ = 0;
			else
				focusedIndex_ += 1;

			for (size_t i = focusedIndex_; i >= 0; i--)
			{
				ControlBase* control = contents_[i].get();
				if (control->IsFocusable() && control->IsTabStop())
				{
					focusedIndex_ = i;
					PushFocus(control);
					return true;
				}
			}

			break;
		}
	}

	return false;
}

Size StackPanel::MeasureOverride(const Size& availableSize)
{
	int32_t totalWidth = 0;
	int32_t totalHeight = 0;
	
	for (const std::unique_ptr<ControlBase>& child : contents_)
	{
		const Size childAvailableSize = Size(availableSize.Width, availableSize.Height - totalHeight);
		const Size childMeasured = child->Measure(childAvailableSize);

		totalWidth = std::max(childMeasured.Width, totalWidth);
		totalHeight += childMeasured.Height;
	}

	return Size(totalWidth, totalHeight);
}

void StackPanel::ArrangeOverride(const Rect& contentRect)
{
	int32_t currentY = 0;

	for (const std::unique_ptr<ControlBase>& child : contents_)
	{
		const Size childSize = child->GetActualSize();
		const Rect childRect(0, currentY, contentRect.Width, childSize.Height);

		child->Arrange(childRect);
		currentY += child->GetArrangedRect().Height;
	}
}

void StackPanel::RenderOverride(RenderContext& context)
{
	for (const std::unique_ptr<ControlBase>& child : contents_)
	{
		Rect childRect = child->GetArrangedRect();
		RenderContext childContext = context.CreateInner(childRect);
		child->Render(childContext);
	}
}
