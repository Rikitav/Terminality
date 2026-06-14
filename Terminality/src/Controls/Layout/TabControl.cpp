#pragma once

#include <algorithm>
#include <vector>
#include <string>
#include <memory>

#include <terminality/Terminality.hpp>

using namespace terminality;

static constexpr bool in_bounds(int index, const std::vector<TabItem>& tabs)
{
	return index >= 0 && index < tabs.size();
}

void TabControl::AddTab(const std::string& header, std::unique_ptr<ControlBase> content)
{
    if (content)
    {
        content->SetParent(this);
        if (!content->IsAttached())
            content->OnAttachedToTree();
    }

    tabs_.emplace_back(header, std::move(content));
    InvalidateMeasure();
}

void TabControl::RemoveTab(int index)
{
    if (index >= 0 && index < tabs_.size())
    {
        if (tabs_[index].Content)
            tabs_[index].Content->SetParent(nullptr);

        tabs_.erase(tabs_.begin() + index);
        if (SelectedIndex.Get() >= tabs_.size())
            SelectedIndex = std::max(0, (int)tabs_.size() - 1);

        InvalidateMeasure();
    }
}

void TabControl::ClearTabs()
{
    for (auto& tab : tabs_)
    {
        if (tab.Content)
            tab.Content->SetParent(nullptr);
    }

    tabs_.clear();
    SelectedIndex = 0;
    InvalidateMeasure();
}

size_t TabControl::GetTabCount() const
{
    return tabs_.size();
}

bool TabControl::OnKeyDown(InputEvent input)
{
    if (tabs_.empty())
        return false;

    if (input.Key == InputKey::LEFT)
    {
        int idx = SelectedIndex.Get();
        if (idx > 0)
        {
            SelectedIndex = idx - 1;
            return true;
        }
    }
    else if (input.Key == InputKey::RIGHT)
    {
        int idx = SelectedIndex.Get();
        if (idx < (int)tabs_.size() - 1)
        {
            SelectedIndex = idx + 1;
            return true;
        }
    }
    else if (input.Key == InputKey::DOWN || input.Key == InputKey::RETURN)
    {
        int idx = SelectedIndex.Get();
        if (in_bounds(idx, tabs_))
        {
            ControlBase* content = tabs_[idx].Content.get();
            if (content != nullptr && content->IsFocusable())
            {
                PushFocus(content);
                return true;
            }
        }
    }

    return ControlBase::OnKeyDown(input);
}

std::size_t TabControl::VisualChildrenCount() const
{
    int idx = SelectedIndex.Get();
    if (in_bounds(idx, tabs_) && tabs_[idx].Content != nullptr)
        return 1;

    return 0;
}

VisualTreeNode* TabControl::GetVisualChild(std::size_t index) const
{
    int idx = SelectedIndex.Get();
    if (in_bounds(idx, tabs_))
    {
        ControlBase* content = tabs_[idx].Content.get();
        if (content != nullptr && index == 0)
            return content;
    }

    return nullptr;
}

bool TabControl::MoveFocusNext(Direction direction, InputModifier modifiers)
{
    if (tabs_.empty())
        return false;

    int idx = SelectedIndex.Get();
    if (in_bounds(idx, tabs_))
    {
        if (direction == Direction::Down || direction == Direction::Next)
        {
            ControlBase* content = tabs_[idx].Content.get();
            if (content != nullptr && content->IsFocusable())
            {
                PushFocus(content);
                return true;
            }
        }
    }

    return false;
}

void TabControl::OnGotFocus()
{
    InvalidateVisual();
}

void TabControl::OnLostFocus()
{
    InvalidateVisual();
}

void TabControl::OnPropertyChanged(const char* propertyName)
{
    ControlBase::OnPropertyChanged(propertyName);
}

Size TabControl::MeasureOverride(const Size& availableSize)
{
    int32_t totalHeaderWidth = 0;
    for (const auto& tab : tabs_)
    {
        totalHeaderWidth += tab.Header.length() + 5; // "[*] Header "
    }

    Size contentAvailable = availableSize;
    if (contentAvailable.Height != -1)
        contentAvailable.Height = std::max(0, contentAvailable.Height - 1);

    int32_t maxContentWidth = 0;
    int32_t maxContentHeight = 0;

    int idx = SelectedIndex.Get();
    if (in_bounds(idx, tabs_))
    {
        ControlBase* content = tabs_[idx].Content.get();
        if (content != nullptr)
        {
            Size childSize = content->Measure(contentAvailable);
            maxContentWidth = childSize.Width;
            maxContentHeight = childSize.Height;
        }
    }

    return Size(
        std::max(totalHeaderWidth, maxContentWidth),
        maxContentHeight + 1
    );
}

void TabControl::ArrangeOverride(const Rect& finalRect)
{
    int idx = SelectedIndex.Get();
    if (in_bounds(idx, tabs_))
    {
        ControlBase* content = tabs_[idx].Content.get();
        if (content != nullptr)
        {
            Rect contentRect(
                finalRect.X,
                finalRect.Y + 1,
                finalRect.Width,
                std::max(0, finalRect.Height - 1));

            content->Arrange(contentRect);
        }
    }
}

void TabControl::RenderOverride(RenderContext& context)
{
    int idx = SelectedIndex.Get();
    Rect bounds = context.ContextRect();

    int32_t currentX = 0;
    for (int i = 0; i < tabs_.size(); ++i)
    {
        const auto& tab = tabs_[i];
        std::string headerText;
        if (i == idx)
        {
            headerText = focused_
                ? ("[>] " + tab.Header + " ")
                : ("[*] " + tab.Header + " ");
        }
        else
        {
            headerText = "[ ] " + tab.Header + " ";
        }

        int width = headerText.length();
        if (currentX + width > bounds.Width)
        {
            std::string trunc = headerText.substr(0, std::max(0, bounds.Width - currentX));
            if (!trunc.empty())
            {
                Color fg = (i == idx)
                    ? FocusedForegroundColor.Get()
                    : ForegroundColor.Get();

                Color bg = (i == idx)
                    ? FocusedBackgroundColor.Get()
                    : BackgroundColor.Get();

                context.RenderText(Point(currentX, 0), trunc, fg, bg);
            }

            break;
        }

        Color fg = (i == idx)
            ? FocusedForegroundColor.Get()
            : ForegroundColor.Get();

        Color bg = (i == idx)
            ? FocusedBackgroundColor.Get()
            : BackgroundColor.Get();

        context.RenderText(Point(currentX, 0), headerText, fg, bg);
        currentX += width;
    }

    if (in_bounds(idx, tabs_))
    {
        ControlBase* content = tabs_[idx].Content.get();
        if (content != nullptr)
        {
            Rect childRect = tabs_[idx].Content->GetArrangedRect();
            RenderContext childContext = context.CreateInner(childRect);
            tabs_[idx].Content->Render(childContext);
        }
    }
}
