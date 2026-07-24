
#include <cstdint>
#include <memory>

#include <terminality/Framework/VisualTreeNode.hpp>
#include <terminality/Engine/DispatchTimer.hpp>
#include <terminality/Engine/FocusManager.hpp>

using namespace terminality;

bool VisualTreeNode::PopFocus(Direction direction, InputModifier modifiers)
{
	return FocusManager::Current().MoveNext(direction, modifiers);
}

void VisualTreeNode::PushFocus(VisualTreeNode* focused)
{
	FocusManager::Current().SetFocused(focused);
}

VisualTreeNode* VisualTreeNode::GetParent() const
{
	return parent_;
}

void VisualTreeNode::InvalidateMeasure()
{
	DispatchTimer::Current().VerifyAccess();
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
	DispatchTimer::Current().VerifyAccess();
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
	DispatchTimer::Current().VerifyAccess();
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
	return isTabStop_;
}

int VisualTreeNode::GetTabIndex() const
{
	return tabIndex_;
}

bool VisualTreeNode::OnKeyDown(InputEvent input)
{
	return false;
}

bool VisualTreeNode::OnKeyUp(InputEvent input)
{
	return false;
}

bool VisualTreeNode::MoveFocusNext(Direction direction, InputModifier modifiers)
{
	if (!IsFocusable())
		return false;
	
	if (!focused_)
		return true;

	return false;
}

void VisualTreeNode::OnChildInvalidated(VisualTreeNode& child)
{
	InvalidateMeasure();
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
	FocusManager::Current().ClearFocus(this);

	VisualTreeNode* parent = parent_;
	parent_ = nullptr;
	layer_ = nullptr;

	if (parent != nullptr)
	{
		parent->InvalidateMeasure();
		parent->InvalidateVisual();
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

Size VisualTreeNode::GetActualSize() const
{
	return actualSize_;
}

Rect VisualTreeNode::GetArrangedRect() const
{
	return arrangedRect_;
}
