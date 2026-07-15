
#include <algorithm>
#include <memory>

#include <terminality/Terminality.hpp>

using namespace terminality;

ControlBase* Heap::AddChild(Point position, std::unique_ptr<ControlBase> child)
{
	if (child == nullptr)
		return nullptr;

	child->SetParent(this);
	if (!child->IsAttached())
		child->OnAttachedToTree();

	ControlBase* raw = child.get();
	children_.push_back(HeapChild{ std::move(child), position });

	InvalidateMeasure();
	return raw;
}

ControlBase* Heap::AddChild(int32_t x, int32_t y, std::unique_ptr<ControlBase> child)
{
	return AddChild(Point(x, y), std::move(child));
}

std::unique_ptr<ControlBase> Heap::RemoveChild(ControlBase* child)
{
	for (auto it = children_.begin(); it != children_.end(); ++it)
	{
		if (it->Control.get() == child)
		{
			auto removed = std::move(it->Control);
			removed->SetParent(nullptr);
			children_.erase(it);

			if (focusedIndex_ >= children_.size())
				focusedIndex_ = children_.empty() ? 0 : children_.size() - 1;

			InvalidateMeasure();
			return removed;
		}
	}

	return nullptr;
}

void Heap::Clear()
{
	for (auto& entry : children_)
	{
		if (entry.Control)
			entry.Control->SetParent(nullptr);
	}

	children_.clear();
	focusedIndex_ = 0;
	InvalidateMeasure();
}

std::size_t Heap::Count() const
{
	return children_.size();
}

std::size_t Heap::VisualChildrenCount() const
{
	return children_.size();
}

VisualTreeNode* Heap::GetVisualChild(std::size_t index) const
{
	return children_.at(index).Control.get();
}

void Heap::OnGotFocus()
{
	if (children_.empty())
	{
		InvalidateVisual();
		return;
	}

	if (focusedIndex_ < children_.size())
	{
		VisualTreeNode* node = children_[focusedIndex_].Control.get();
		if (node->IsFocusable())
		{
			PushFocus(node);
			InvalidateVisual();
			return;
		}
	}

	for (std::size_t i = 0; i < children_.size(); ++i)
	{
		VisualTreeNode* node = children_[i].Control.get();
		if (node->IsFocusable())
		{
			focusedIndex_ = i;
			PushFocus(node);
			break;
		}
	}

	InvalidateVisual();
}

void Heap::OnLostFocus()
{
	focused_ = false;
	InvalidateVisual();
}

bool Heap::MoveFocusNext(Direction direction, InputModifier /*modifiers*/)
{
	if (children_.empty())
		return false;

	bool forward = (direction == Direction::Next || direction == Direction::Down || direction == Direction::Right);
	bool backward = (direction == Direction::Previous || direction == Direction::Up || direction == Direction::Left);

	if (!forward && !backward)
		return false;

	if (forward)
	{
		for (std::size_t i = focusedIndex_ + 1; i < children_.size(); ++i)
		{
			ControlBase* node = children_[i].Control.get();
			if (node->IsFocusable())
			{
				focusedIndex_ = i;
				PushFocus(node);
				return true;
			}
		}
	}
	else
	{
		for (std::size_t i = focusedIndex_; i-- > 0;)
		{
			ControlBase* node = children_[i].Control.get();
			if (node->IsFocusable())
			{
				focusedIndex_ = i;
				PushFocus(node);
				return true;
			}
		}
	}

	return false;
}

Size Heap::MeasureOverride(const Size& availableSize)
{
	int32_t maxWidth = 0;
	int32_t maxHeight = 0;

	for (auto& entry : children_)
	{
		Size childSize = entry.Control->Measure(availableSize);

		int32_t right = entry.Position.X + (childSize.Width < 0 ? 0 : childSize.Width);
		int32_t bottom = entry.Position.Y + (childSize.Height < 0 ? 0 : childSize.Height);

		maxWidth = std::max(maxWidth, right);
		maxHeight = std::max(maxHeight, bottom);
	}

	if (availableSize.Width >= 0)
		maxWidth = std::min(maxWidth, availableSize.Width);

	if (availableSize.Height >= 0)
		maxHeight = std::min(maxHeight, availableSize.Height);

	return Size(maxWidth, maxHeight);
}

void Heap::ArrangeOverride(const Rect& contentRect)
{
	for (auto& entry : children_)
	{
		Size childSize = entry.Control->GetActualSize();

		Rect childRect(
			contentRect.X + entry.Position.X,
			contentRect.Y + entry.Position.Y,
			childSize.Width < 0 ? 0 : childSize.Width,
			childSize.Height < 0 ? 0 : childSize.Height);

		entry.Control->Arrange(childRect);
	}
}

void Heap::RenderOverride(RenderContext& context)
{
	for (auto& entry : children_)
	{
		Rect childRect = entry.Control->GetArrangedRect();
		if (context.ContextRect().Intersects(childRect))
		{
			RenderContext childContext = context.CreateInner(childRect);
			entry.Control->Render(childContext);
		}
	}
}
