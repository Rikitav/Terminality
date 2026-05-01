module terminality;

import std;
import std.compat;

using namespace terminality;

VisualTreeNode* VisualTreeNode::GetParent() const
{
	return parent_;
}

void VisualTreeNode::InvalidateMeasure()
{
	measureDirty_ = true;
	arrangeDirty_ = true;
	visualDirty_ = true;
	
	if (parent_ != nullptr)
	{
		parent_->OnChildInvalidated(*this);
		parent_->InvalidateMeasure();
	}
}

void VisualTreeNode::InvalidateArrange()
{
	arrangeDirty_ = true;
	visualDirty_ = true;
	
	if (parent_ != nullptr)
	{
		parent_->OnChildInvalidated(*this);
		parent_->InvalidateArrange();
	}
}

void VisualTreeNode::InvalidateVisual()
{
	visualDirty_ = true;
	
	if (parent_ != nullptr)
	{
		//parent_->OnChildInvalidated(*this);
		parent_->InvalidateVisual();
	}
}

bool VisualTreeNode::IsAttached() const
{
	return attached_;
}

bool VisualTreeNode::IsMeasureDirty() const
{
	return measureDirty_;
}

bool VisualTreeNode::IsArrangeDirty() const
{
	return arrangeDirty_;
}

bool VisualTreeNode::IsVisualDirty() const
{
	return visualDirty_;
}

bool VisualTreeNode::IsFocusable() const
{
	return focusable_;
}

bool VisualTreeNode::IsTabStop() const
{
	return isTabStop_ || true;
}

int VisualTreeNode::GetTabIndex() const
{
	return tabIndex_;
}

bool VisualTreeNode::MoveFocusNext(NavigationDirection direction)
{
	if (!focusable_)
	{
		return false;
	}
	
	if (!focused_)
	{
		OnGotFocus();
		return true;
	}

	OnLostFocus();
	return false;
}

void VisualTreeNode::OnChildInvalidated(VisualTreeNode& child)
{
	InvalidateMeasure();
	InvalidateVisual();
}

void VisualTreeNode::OnAttachedToTree()
{
	attached_ = true;
	InvalidateMeasure();
	InvalidateVisual();
}

void VisualTreeNode::OnDettachedFromTree()
{
	attached_ = false;
	InvalidateMeasure();
	InvalidateVisual();
}

void VisualTreeNode::OnKeyDown(InputEvent input)
{
	return;
}

void VisualTreeNode::OnKeyUp(InputEvent input)
{
	switch (input.Key)
	{
		case InputKey::None:
		{
			break;
		}

		case InputKey::UP:
		{
			FocusManager::Current().MoveNext(NavigationDirection::Up);
			break;
		}

		case InputKey::DOWN:
		{
			FocusManager::Current().MoveNext(NavigationDirection::Down);
			break;
		}

		case InputKey::LEFT:
		{
			FocusManager::Current().MoveNext(NavigationDirection::Left);
			break;
		}

		case InputKey::RIGHT:
		{
			FocusManager::Current().MoveNext(NavigationDirection::Right);
			break;
		}

		case InputKey::TAB:
		{
			FocusManager::Current().MoveNext(hasFlag(input.Modifier, InputModifier::Shift)
				? NavigationDirection::Previous : NavigationDirection::Next);
			break;
		}

		default:
		{
			// TODO: Emit input event to focused control
			break;
		}
	}
}

void VisualTreeNode::OnGotFocus()
{
	focused_ = true;
	InvalidateVisual();
}

void VisualTreeNode::OnLostFocus()
{
	focused_ = false;
	InvalidateVisual();
}
