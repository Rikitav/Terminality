#include <cstdint>
#include <memory>
#include <algorithm>
#include <functional>
#include <vector>

#include <terminality/Controls/StackPanel.hpp>

using namespace terminality;

void StackPanel::AddChild(std::unique_ptr<ControlBase> child)
{
    if (child == nullptr)
        return;

    child->SetParent(this);

    contents_.push_back(std::move(child));

    if (AutoScrollToEnd.Get())
    {
        forceScrollToEnd_ = true;
        focusedIndex_ = contents_.size() - 1;
    }

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
        removed->SetParent(nullptr);

    if (focusedIndex_ >= contents_.size())
        focusedIndex_ = contents_.empty() ? 0 : contents_.size() - 1;

    InvalidateMeasure();
    return removed;
}

void StackPanel::Insert(std::size_t index, std::unique_ptr<ControlBase> child)
{
    if (child == nullptr)
        return;

    if (index > contents_.size())
        index = contents_.size();

    child->SetParent(this);

    std::size_t oldSize = contents_.size();
    contents_.insert(contents_.begin() + index, std::move(child));

    if (focusedIndex_ >= index && focusedIndex_ < oldSize)
        focusedIndex_++;

    if (AutoScrollToEnd.Get())
    {
        forceScrollToEnd_ = true;
        focusedIndex_ = contents_.size() - 1;
    }

    InvalidateMeasure();
}

std::unique_ptr<ControlBase> StackPanel::RemoveAt(std::size_t index)
{
    if (index >= contents_.size())
        return nullptr;

    std::unique_ptr<ControlBase> removed = std::move(contents_[index]);
    contents_.erase(contents_.begin() + index);

    if (removed)
        removed->SetParent(nullptr);

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
            child->SetParent(nullptr);
    }

    contents_.clear();
    focusedIndex_ = 0;
    scrollOffset_ = 0;
    InvalidateMeasure();
}

void StackPanel::ScrollIntoView()
{
    if (!Scrollable || contents_.empty() || focusedIndex_ >= contents_.size())
        return;

    int32_t spacing = std::max(0, ItemSpacing.Get());
    int32_t childStart = 0;
    int32_t childSize = 0;

    for (std::size_t i = 0; i <= focusedIndex_; ++i)
    {
        Size sz = contents_[i]->GetActualSize();
        if (i == focusedIndex_)
        {
            childSize = (ContentOrientation == Orientation::Vertical) ? sz.Height : sz.Width;
            break;
        }

        childStart += (ContentOrientation == Orientation::Vertical) ? sz.Height : sz.Width;
        childStart += spacing;
    }

    int32_t viewSize = (ContentOrientation == Orientation::Vertical) ? GetArrangedRect().Height : GetArrangedRect().Width;
    if (childStart < scrollOffset_)
    {
        scrollOffset_ = childStart;
        InvalidateArrange();
    }
    else if (childStart + childSize > scrollOffset_ + viewSize)
    {
        scrollOffset_ = childStart + childSize - viewSize;
        InvalidateArrange();
    }
}

void StackPanel::OnGotFocus()
{
    if (contents_.empty())
    {
        InvalidateVisual();
        return;
    }

    if (focusedIndex_ < contents_.size())
    {
        VisualTreeNode* focusedControl = contents_[focusedIndex_].get();
        if (focusedControl->IsFocusable())
        {
            PushFocus(focusedControl);
            ScrollIntoView();
            InvalidateVisual();
            return;
        }
    }

    for (std::size_t i = 0; i < contents_.size(); ++i)
    {
        VisualTreeNode* focusedControl = contents_[i].get();
        if (focusedControl->IsFocusable())
        {
            focusedIndex_ = i;
            PushFocus(focusedControl);
            ScrollIntoView();
            break;
        }
    }

    InvalidateVisual();
}

void StackPanel::OnLostFocus()
{
    focused_ = false;
    InvalidateVisual();
}

size_t StackPanel::VisualChildrenCount() const
{
    return contents_.size();
}

VisualTreeNode* StackPanel::GetVisualChild(std::size_t index) const
{
    return contents_.at(index).get();
}

void StackPanel::OnPropertyChanged(const char* propertyName)
{
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
                ScrollIntoView();
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

        for (std::size_t i = focusedIndex_; i < contents_.size(); i++)
        {
            ControlBase* control = contents_[i].get();
            if (control->IsFocusable() && control->IsTabStop())
            {
                focusedIndex_ = i;
                PushFocus(control);
                ScrollIntoView();
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

    int32_t spacing = std::max(0, ItemSpacing.Get());
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

    if (!contents_.empty())
    {
        if (ContentOrientation == Orientation::Vertical)
            totalHeight += static_cast<int32_t>(contents_.size() - 1) * spacing;
        else
            totalWidth += static_cast<int32_t>(contents_.size() - 1) * spacing;
    }

    return Size(totalWidth, totalHeight);
}

void StackPanel::ArrangeOverride(const Rect& contentRect)
{
    int32_t totalContentWidth = 0;
    int32_t totalContentHeight = 0;

    int32_t spacing = std::max(0, ItemSpacing.Get());
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

    if (!contents_.empty())
    {
        if (ContentOrientation == Orientation::Vertical)
            totalContentHeight += static_cast<int32_t>(contents_.size() - 1) * spacing;
        else
            totalContentWidth += static_cast<int32_t>(contents_.size() - 1) * spacing;
    }

    if (Scrollable)
    {
        int32_t maxScroll = std::max(0, (ContentOrientation == Orientation::Vertical)
            ? (totalContentHeight - contentRect.Height)
            : (totalContentWidth - contentRect.Width));
        
        if (AutoScrollToEnd.Get() && forceScrollToEnd_)
        {
            scrollOffset_ = maxScroll;
            forceScrollToEnd_ = false;
        }

        scrollOffset_ = std::clamp(scrollOffset_, 0, maxScroll);
    }
    else
    {
        scrollOffset_ = 0;
    }

    int32_t startX = 0;
    int32_t startY = 0;

    if (ContentOrientation == Orientation::Vertical)
    {
        if (VerticalContentAlignment == VerticalAlign::Bottom)
            startY = std::max(0, contentRect.Height - totalContentHeight);
        else if (VerticalContentAlignment == VerticalAlign::Center)
            startY = std::max(0, contentRect.Height - totalContentHeight) / 2;
    }
    else
    {
        if (HorizontalContentAlignment == HorizontalAlign::Right)
            startX = std::max(0, contentRect.Width - totalContentWidth);
        else if (HorizontalContentAlignment == HorizontalAlign::Center)
            startX = std::max(0, contentRect.Width - totalContentWidth) / 2;
    }

    int32_t currentX = startX - (ContentOrientation == Orientation::Horizontal ? scrollOffset_ : 0);
    int32_t currentY = startY - (ContentOrientation == Orientation::Vertical ? scrollOffset_ : 0);

    for (std::size_t i = 0; i < contents_.size(); ++i)
    {
        const std::unique_ptr<ControlBase>& child = contents_[i];
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
            if (i + 1 < contents_.size())
                currentY += spacing;
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
            if (i + 1 < contents_.size())
                currentX += spacing;
        }
    }
}

void StackPanel::RenderOverride(RenderContext& context)
{
    Rect allowed = context.ContextRect();

    for (const std::unique_ptr<ControlBase>& child : contents_)
    {
        Rect childRect = child->GetArrangedRect();
        if (allowed.Intersects(childRect))
        {
            RenderContext childContext = context.CreateInner(childRect);
            child->Render(childContext);
        }
    }

    if (Scrollable)
    {
        int32_t spacing = std::max(0, ItemSpacing.Get());
        int32_t totalContentWidth = 0;
        int32_t totalContentHeight = 0;

        for (const auto& child : contents_)
        {
            const Size childSize = child->GetActualSize();
            if (ContentOrientation == Orientation::Vertical)
                totalContentHeight += childSize.Height;
            else
                totalContentWidth += childSize.Width;
        }

        if (!contents_.empty())
        {
            if (ContentOrientation == Orientation::Vertical)
                totalContentHeight += static_cast<int32_t>(contents_.size() - 1) * spacing;
            else
                totalContentWidth += static_cast<int32_t>(contents_.size() - 1) * spacing;
        }

        if (ContentOrientation == Orientation::Vertical)
        {
            int viewHeight = GetArrangedRect().Height;
            if (totalContentHeight > viewHeight && viewHeight >= 3)
            {
                int maxScroll = totalContentHeight - viewHeight;
                float progress = (float)scrollOffset_ / maxScroll;
                int scrollArea = viewHeight - 2;
                int indicatorPos = 1 + (int)(progress * (scrollArea - 1));

                int x = GetArrangedRect().Width - 1;
                context.SetCell(x, 0, L'^');
                context.SetCell(x, viewHeight - 1, L'v');

                for (int i = 1; i < viewHeight - 1; i++)
                {
                    if (i == indicatorPos)
                        context.SetCell(x, i, L'#');
                    else
                        context.SetCell(x, i, L'|');
                }
            }
        }
        else
        {
            int viewWidth = GetArrangedRect().Width;
            if (totalContentWidth > viewWidth && viewWidth >= 3)
            {
                int maxScroll = totalContentWidth - viewWidth;
                float progress = (float)scrollOffset_ / maxScroll;
                int scrollArea = viewWidth - 2;
                int indicatorPos = 1 + (int)(progress * (scrollArea - 1));

                int y = GetArrangedRect().Height - 1;
                context.SetCell(0, y, L'<');
                context.SetCell(viewWidth - 1, y, L'>');

                for (int i = 1; i < viewWidth - 1; i++)
                {
                    if (i == indicatorPos)
                        context.SetCell(i, y, L'#');
                    else
                        context.SetCell(i, y, L'-');
                }
            }
        }
    }
}
