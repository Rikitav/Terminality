module terminality;

import std;
import std.compat;

using namespace terminality;

void StackPanel::AddChild(std::unique_ptr<ControlBase> child)
{
	if (!child)
		return;

	child->SetParent(this, layer_);
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
		removed->SetParent(nullptr, nullptr);

	if (focusedIndex_ >= contents_.size())
		focusedIndex_ = contents_.empty() ? 0 : contents_.size() - 1;

	InvalidateMeasure();
	return removed;
}

void StackPanel::Insert(size_t index, std::unique_ptr<ControlBase> child)
{
	if (!child)
		return;

	if (index > contents_.size())
		index = contents_.size();

	child->SetParent(this, layer_);
	if (!child->IsAttached())
		child->OnAttachedToTree();

	contents_.insert(contents_.begin() + index, std::move(child));

    if (focusedIndex_ >= index && focusedIndex_ < contents_.size() - 1)
        focusedIndex_++;
    else
        PushFocus(child.get());

	InvalidateMeasure();
}

std::unique_ptr<ControlBase> StackPanel::RemoveAt(size_t index)
{
	if (index >= contents_.size())
		return nullptr;

	std::unique_ptr<ControlBase> removed = std::move(contents_[index]);
	contents_.erase(contents_.begin() + index);

	if (removed)
		removed->SetParent(nullptr, nullptr);

	if (focusedIndex_ >= contents_.size())
		focusedIndex_ = contents_.empty() ? 0 : contents_.size() - 1;

	InvalidateMeasure();
	return removed;
}

void StackPanel::Clear()
{
	for (auto& child : contents_)
	{
		if (child)
			child->SetParent(nullptr, nullptr);
	}
	
	contents_.clear();
	focusedIndex_ = 0;
	InvalidateMeasure();
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

void StackPanel::OnPropertyChanged(const char* propertyName)
{
	/*
	if (std::strcmp(propertyName, "Content") == 0)
	{
		InvalidateMeasure();
		InvalidateVisual();
		return;
	}
	*/

	ControlBase::OnPropertyChanged(propertyName);
}

bool StackPanel::MoveFocusNext(Direction direction, InputModifier modifiers)
{
    if (contents_.empty())
        return false;

    bool goBack = false;
    bool goForward = false;

    switch (direction)
    {
        case Direction::Previous:
        {
            goBack = true;
            break;
        }

        case Direction::Next:
        {
            goForward = true;
            break;
        }

        case Direction::Up:
        {
            if (ContentOrientation == Orientation::Vertical)
                goBack = true;

            break;
        }

        case Direction::Down:
        {
            if (ContentOrientation == Orientation::Vertical)
                goForward = true;

            break;
        }

        case Direction::Left:
        {
            if (ContentOrientation == Orientation::Horizontal)
                goBack = true;

            break;
        }

        case Direction::Right:
        {
            if (ContentOrientation == Orientation::Horizontal)
                goForward = true;

            break;
        }
    }

    size_t focusedIndexPre = focusedIndex_;
    if (goBack)
    {
        if (focusedIndex_ == 0)
        {
            if (Looping)
            {
                if (hasFlag(modifiers, InputModifier::LeftAlt) || hasFlag(modifiers, InputModifier::RightAlt))
                    return false;

                focusedIndex_ = contents_.size() - 1;
            }
            else
            {
                return false;
            }
        }
        else
        {
            focusedIndex_ -= 1;
        }

        for (int i = static_cast<int>(focusedIndex_); i >= 0; i--)
        {
            ControlBase* control = contents_[i].get();
            if (control->IsFocusable() && control->IsTabStop())
            {
                focusedIndex_ = i;
                PushFocus(control);
                return true;
            }
        }
    }
    else if (goForward)
    {
        if (focusedIndex_ == contents_.size() - 1)
        {
            if (Looping)
            {
                if (hasFlag(modifiers, InputModifier::LeftAlt) || hasFlag(modifiers, InputModifier::RightAlt))
                    return false;

                focusedIndex_ = 0;
            }
            else
            {
                return false;
            }
        }
        else
        {
            focusedIndex_ += 1;
        }
            
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
    }

    focusedIndex_ = focusedIndexPre;
    return false;
}

Size StackPanel::MeasureOverride(const Size& availableSize)
{
    int32_t totalWidth = 0;
    int32_t totalHeight = 0;

    for (const auto& child : contents_)
    {
        Size childAvailable = availableSize;
        if (ContentOrientation == Orientation::Vertical)
            childAvailable.Height = -1;
        else
            childAvailable.Width = -1;

        const Size childSize = child->Measure(childAvailable);
        if (ContentOrientation == Orientation::Vertical)
        {
            totalWidth = std::max(childSize.Width, totalWidth);
            totalHeight += childSize.Height;
        }
        else
        {
            totalWidth += childSize.Width;
            totalHeight = std::max(childSize.Height, totalHeight);
        }
    }

    return Size(totalWidth, totalHeight);
}

void StackPanel::ArrangeOverride(const Rect& contentRect)
{
    int32_t totalContentWidth = 0;
    int32_t totalContentHeight = 0;

    for (const auto& child : contents_)
    {
        const Size childSize = child->GetActualSize();
        if (ContentOrientation == Orientation::Vertical)
        {
            totalContentWidth = std::max(totalContentWidth, childSize.Width);
            totalContentHeight += childSize.Height;
        }
        else
        {
            totalContentWidth += childSize.Width;
            totalContentHeight = std::max(totalContentHeight, childSize.Height);
        }
    }

    int32_t startX = 0;
    int32_t startY = 0;

    if (ContentOrientation == Orientation::Vertical)
    {
        if (VerticalContentAlignment == VerticalAlignment::Bottom)
            startY = std::max(0, contentRect.Height - totalContentHeight);
        else if (VerticalContentAlignment == VerticalAlignment::Center)
            startY = std::max(0, contentRect.Height - totalContentHeight) / 2;
    }
    else
    {
        if (HorizontalContentAlignment == HorizontalAlignment::Right)
            startX = std::max(0, contentRect.Width - totalContentWidth);
        else if (HorizontalContentAlignment == HorizontalAlignment::Center)
            startX = std::max(0, contentRect.Width - totalContentWidth) / 2;
    }

    int32_t currentX = startX;
    int32_t currentY = startY;

    for (const std::unique_ptr<ControlBase>& child : contents_)
    {
        const Size childSize = child->GetActualSize();

        if (ContentOrientation == Orientation::Vertical)
        {
            const Rect childRect(
                contentRect.X,
                contentRect.Y + currentY,
                contentRect.Width,
                childSize.Height);

            child->Arrange(childRect);
            currentY += childSize.Height;
        }
        else
        {
            const Rect childRect(
                contentRect.X + currentX,
                contentRect.Y,
                childSize.Width,
                contentRect.Height);

            child->Arrange(childRect);
            currentX += childSize.Width;
        }
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
