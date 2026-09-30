
#include <algorithm>
#include <cmath>
#include <vector>

#include <terminality/Controls/ResizePanel.hpp>
#include <terminality/Engine/FocusManager.hpp>
#include <terminality/Engine/RenderContext.hpp>

using namespace terminality;

namespace
{
    constexpr InputKey KeyPool[] =
    {
        InputKey::Q, InputKey::W, InputKey::E, InputKey::R, InputKey::T, InputKey::Y,
        InputKey::U, InputKey::I, InputKey::O, InputKey::P, InputKey::A, InputKey::S,
        InputKey::D, InputKey::F, InputKey::G, InputKey::H, InputKey::J, InputKey::K,
        InputKey::L, InputKey::Z, InputKey::X, InputKey::C, InputKey::V, InputKey::B,
        InputKey::N, InputKey::M
    };

    constexpr std::size_t KeyPoolSize = sizeof(KeyPool) / sizeof(KeyPool[0]);

    static bool IsAutoAssignBlocked(InputKey key)
    {
        return key == InputKey::LEFT || key == InputKey::RIGHT ||
               key == InputKey::UP || key == InputKey::DOWN;
    }

    static InputKey GeneratedKey(std::size_t slot)
    {
        for (std::size_t attempt = 0; attempt < KeyPoolSize; ++attempt)
        {
            const InputKey key = KeyPool[(slot + attempt) % KeyPoolSize];
            if (!IsAutoAssignBlocked(key))
                return key;
        }

        return InputKey::None;
    }

    static std::wstring KeyToText(InputKey key)
    {
        if (key >= InputKey::A && key <= InputKey::Z)
            return std::wstring(1, static_cast<wchar_t>(L'A' + (static_cast<int>(key) - static_cast<int>(InputKey::A))));

        if (key >= InputKey::NUM0 && key <= InputKey::NUM9)
            return std::wstring(1, static_cast<wchar_t>(L'0' + (static_cast<int>(key) - static_cast<int>(InputKey::NUM0))));

        switch (key)
        {
            case InputKey::LEFT:    return L"\u2190";
            case InputKey::RIGHT:   return L"\u2192";
            case InputKey::UP:      return L"\u2191";
            case InputKey::DOWN:    return L"\u2193";
            case InputKey::RETURN:  return L"Enter";
            case InputKey::SPACE:   return L"Space";
            case InputKey::ESCAPE:  return L"Esc";
            case InputKey::TAB:     return L"Tab";
            case InputKey::HOME:    return L"Home";
            case InputKey::END:     return L"End";
            case InputKey::PRIOR:   return L"PgUp";
            case InputKey::NEXT:    return L"PgDn";
            case InputKey::INSERT:  return L"Ins";
            case KEY_DELETE:        return L"Del";
            default:                return L"?";
        }
    }

    static bool IsFocusablePanelContent(VisualTreeNode* node)
    {
        if (node == nullptr)
            return false;

        if (auto* control = dynamic_cast<ControlBase*>(node))
        {
            if (control->IsFocusable() && control->IsTabStop())
            {
                if (node->VisualChildrenCount() == 0)
                    return true;

                for (std::size_t i = 0; i < node->VisualChildrenCount(); ++i)
                {
                    if (IsFocusablePanelContent(node->GetVisualChild(i)))
                        return true;
                }

                return false;
            }
        }

        for (std::size_t i = 0; i < node->VisualChildrenCount(); ++i)
        {
            if (IsFocusablePanelContent(node->GetVisualChild(i)))
                return true;
        }

        return false;
    }

    static VisualTreeNode* FindFirstFocusablePanelDescendant(VisualTreeNode* node)
    {
        if (node == nullptr)
            return nullptr;

        if (auto* control = dynamic_cast<ControlBase*>(node))
        {
            if (control->IsFocusable() && control->IsTabStop())
                return node;
        }

        for (std::size_t i = 0; i < node->VisualChildrenCount(); ++i)
        {
            if (VisualTreeNode* found = FindFirstFocusablePanelDescendant(node->GetVisualChild(i)))
                return found;
        }

        return nullptr;
    }

    static void FocusPanelContent(ControlBase* content)
    {
        if (content == nullptr)
            return;

        if (content->IsFocusable() && content->IsTabStop())
        {
            FocusManager::Current().SetFocused(content);
            return;
        }

        if (VisualTreeNode* target = FindFirstFocusablePanelDescendant(content))
            FocusManager::Current().SetFocused(target);
    }
}

// ------------------------------------------------------------------
// ResizePanel
// ------------------------------------------------------------------

ResizePanel::ResizePanel()
{

}

void ResizePanel::SyncDividers()
{
    while (dividers_.size() + 1 < children_.size())
        dividers_.emplace_back();

    while (!dividers_.empty() && dividers_.size() + 1 > children_.size())
        dividers_.pop_back();

    if (focusedIndex_ >= children_.size())
        focusedIndex_ = children_.empty() ? 0 : children_.size() - 1;

    for (std::size_t i = 0; i < dividers_.size(); ++i)
    {
        if (dividers_[i].CustomKeys)
            continue;

        dividers_[i].RetractKey = GeneratedKey(2 * i);
        dividers_[i].ExpandKey = GeneratedKey(2 * i + 1);
    }
}

bool ResizePanel::IsVertical() const
{
    return Orientation.Get() == terminality::Orientation::Vertical;
}

bool ResizePanel::IsShiftActive(const InputEvent& input) const
{
    return shiftHeld_ || hasFlag(input.Modifier, InputModifier::Shift);
}

void ResizePanel::AddChildControl(std::unique_ptr<ControlBase> child)
{
    if (!child)
        return;

    child->SetParent(this);
    if (!child->IsAttached())
        child->OnAttachedToTree();

    children_.push_back(std::move(child));
    SyncDividers();
    InvalidateMeasure();
}

std::unique_ptr<ControlBase> ResizePanel::RemoveChildControl(ControlBase* child)
{
    if (child == nullptr)
        return nullptr;

    for (auto it = children_.begin(); it != children_.end(); ++it)
    {
        if (it->get() == child)
        {
            std::unique_ptr<ControlBase> removed = std::move(*it);
            children_.erase(it);
            SyncDividers();
            InvalidateMeasure();
            return removed;
        }
    }

    return nullptr;
}

std::unique_ptr<ControlBase> ResizePanel::RemoveAt(std::size_t index)
{
    if (index >= children_.size())
        return nullptr;

    std::unique_ptr<ControlBase> removed = std::move(children_[index]);
    children_.erase(children_.begin() + static_cast<std::ptrdiff_t>(index));
    SyncDividers();
    InvalidateMeasure();
    return removed;
}

void ResizePanel::Clear()
{
    children_.clear();
    SyncDividers();
    InvalidateMeasure();
}

std::size_t ResizePanel::GetChildCount() const
{
    return children_.size();
}

std::size_t ResizePanel::GetDividerCount() const
{
    return dividers_.size();
}

void ResizePanel::SetSplitterKeys(std::size_t index, InputKey retractKey, InputKey expandKey)
{
    if (index >= dividers_.size())
        return;

    dividers_[index].RetractKey = retractKey;
    dividers_[index].ExpandKey = expandKey;
    dividers_[index].CustomKeys = true;
    InvalidateVisual();
}

void ResizePanel::MoveSplitter(std::size_t index, int32_t delta)
{
    if (index >= dividers_.size() || delta == 0)
        return;

    dividers_[index].Offset += delta;
    InvalidateArrange();
    SplitterMoved.Emit(index);
}

int32_t ResizePanel::GetSplitterOffset(std::size_t index) const
{
    if (index >= dividers_.size())
        return 0;

    return dividers_[index].Offset;
}

bool ResizePanel::OnKeyDown(InputEvent input)
{
    if (input.Key == InputKey::SHIFT)
    {
        if (input.Pressed && !shiftHeld_)
        {
            shiftHeld_ = true;
            InvalidateVisual();
        }
    }
    else if (input.Pressed && IsShiftActive(input))
    {
        const int32_t expandDelta = IsVertical() ? -1 : +1;

        for (std::size_t i = 0; i < dividers_.size(); ++i)
        {
            if (input.Key == dividers_[i].ExpandKey)
            {
                pressedDivider_ = i;
                lastSplitterPress_ = std::chrono::steady_clock::now();
                InvalidateVisual();
                MoveSplitter(i, expandDelta);
                return true;
            }

            if (input.Key == dividers_[i].RetractKey)
            {
                pressedDivider_ = i;
                lastSplitterPress_ = std::chrono::steady_clock::now();
                InvalidateVisual();
                MoveSplitter(i, -expandDelta);
                return true;
            }
        }
    }

    return ControlBase::OnKeyDown(input);
}

bool ResizePanel::OnKeyUp(InputEvent input)
{
    if (input.Key == InputKey::SHIFT && !input.Pressed && shiftHeld_)
    {
        shiftHeld_ = false;
        InvalidateVisual();
    }

    if (pressedDivider_ != static_cast<std::size_t>(-1))
    {
        const bool shiftReleased = input.Key == InputKey::SHIFT;
        const bool keyReleased = pressedDivider_ < dividers_.size() &&
            (input.Key == dividers_[pressedDivider_].ExpandKey ||
             input.Key == dividers_[pressedDivider_].RetractKey);

        if (shiftReleased || keyReleased)
        {
            pressedDivider_ = static_cast<std::size_t>(-1);
            InvalidateVisual();
        }
    }

    return ControlBase::OnKeyUp(input);
}

Size ResizePanel::MeasureOverride(const Size& availableSize)
{
    const std::size_t n = children_.size();
    if (n == 0)
        return Size(0, 0);

    const int32_t gaps = static_cast<int32_t>(n - 1);
    const bool vertical = IsVertical();

    int32_t maxCross = 0;
    int32_t totalMain = 0;

    for (const auto& child : children_)
    {
        const Size childSize = vertical
            ? child->Measure(Size(availableSize.Width, -1))
            : child->Measure(Size(-1, availableSize.Height));

        if (vertical)
        {
            maxCross = std::max(maxCross, childSize.Width);
            totalMain += childSize.Height;
        }
        else
        {
            totalMain += childSize.Width;
            maxCross = std::max(maxCross, childSize.Height);
        }
    }

    return vertical ? Size(maxCross, totalMain + gaps) : Size(totalMain + gaps, maxCross);
}

void ResizePanel::ArrangeOverride(const Rect& finalRect)
{
    const std::size_t n = children_.size();
    dividerPositions_.clear();

    if (n == 0)
        return;

    const bool vertical = IsVertical();
    const int32_t mainLen = vertical ? finalRect.Height : finalRect.Width;
    const int32_t m = static_cast<int32_t>(n) - 1;
    const int32_t usable = mainLen - m;   // Space left after reserving one cell per divider

    std::vector<int32_t> positions(static_cast<std::size_t>(m));
    int32_t previous = -1;   // Virtual divider in front of the first child

    for (int32_t j = 0; j < m; ++j)
    {
        const int32_t base = static_cast<int32_t>(std::lround((j + 1) * static_cast<double>(usable) / static_cast<double>(n))) + j;
        const int32_t clamped = std::max(base + dividers_[static_cast<std::size_t>(j)].Offset, previous + 2);
        
        positions[static_cast<std::size_t>(j)] = clamped;
        previous = clamped;
    }

    for (int32_t j = m - 1; j >= 0; --j)
    {
        const int32_t maxPos = mainLen - 2 * (m - j);
        positions[static_cast<std::size_t>(j)] = std::min(positions[static_cast<std::size_t>(j)], maxPos);
    }

    int32_t start = 0;
    for (std::size_t i = 0; i < n; ++i)
    {
        const int32_t end = (i < static_cast<std::size_t>(m)) ? positions[i] : mainLen;
        const Rect childRect = vertical
            ? Rect(finalRect.X, finalRect.Y + start, finalRect.Width, end - start)
            : Rect(finalRect.X + start, finalRect.Y, end - start, finalRect.Height);

        if (childRect.Width > 0 && childRect.Height > 0)
            children_[i]->Arrange(childRect);

        start = end + 1;   // Skip the divider cell
    }

    dividerPositions_ = std::move(positions);
}

void ResizePanel::RenderOverride(RenderContext& context)
{
    for (const auto& child : children_)
    {
        RenderContext childContext = context.CreateInner(child->GetArrangedRect());
        child->Render(childContext);
    }

    const Rect rect = context.ContextRect();
    if (dividerPositions_.size() != dividers_.size())
        return;

    const bool vertical = IsVertical();
    const Color lineColor = (shiftHeld_ ? FocusedSplitterColor.Get() : SplitterColor.Get());

    // Divider lines
    for (std::size_t j = 0; j < dividerPositions_.size(); ++j)
    {
        const int32_t pos = dividerPositions_[j];

        if (vertical)
        {
            for (int32_t x = 0; x < rect.Width; ++x)
                context.SetCell(x, pos, L'\u2500', lineColor, Color::TRANSPARENT);
        }
        else
        {
            for (int32_t y = 0; y < rect.Height; ++y)
                context.SetCell(pos, y, L'\u2502', lineColor, Color::TRANSPARENT);
        }
    }

    // Hint tags while SHIFT is held
    if (!shiftHeld_ || !ShowHints.Get())
        return;

    for (std::size_t j = 0; j < dividers_.size(); ++j)
    {
        const bool pressed = j == pressedDivider_;
        const Color hintFore = pressed ? PressedForegroundColor.Get() : FocusedForegroundColor.Get();
        const Color hintBack = pressed ? PressedBackgroundColor.Get() : FocusedBackgroundColor.Get();

        const std::wstring text = L"SHIFT+" + KeyToText(dividers_[j].RetractKey) + L"\\" + KeyToText(dividers_[j].ExpandKey);

        const int32_t boxW = static_cast<int32_t>(text.size()) + 2;
        const int32_t boxH = 3;

        if (rect.Width < boxW || rect.Height < boxH)
            continue;

        const int32_t centerX = vertical ? rect.Width / 2 : dividerPositions_[j];
        const int32_t centerY = vertical ? dividerPositions_[j] : rect.Height / 2;

        const int32_t x0 = std::clamp(centerX - boxW / 2, 0, rect.Width - boxW);
        const int32_t y0 = std::clamp(centerY - boxH / 2, 0, rect.Height - boxH);

        for (int32_t dy = 0; dy < boxH; ++dy)
        {
            for (int32_t dx = 0; dx < boxW; ++dx)
            {
                wchar_t glyph = L' ';

                if (dy == 0)
                {
                    glyph = (dx == 0) ? L'\u256D'
                        : (dx == boxW - 1) ? L'\u256E'
                        : L'\u2500';
                }
                else if (dy == boxH - 1)
                {
                    glyph = (dx == 0) ? L'\u2570'
                        : (dx == boxW - 1) ? L'\u256F'
                        : L'\u2500';
                }
                else
                {
                    glyph = (dx == 0 || dx == boxW - 1) ? L'\u2502'
                        : L' ';
                }

                context.SetCell(x0 + dx, y0 + dy, glyph, hintFore, hintBack);
            }
        }

        context.RenderText(Point(x0 + 1, y0 + 1), text, hintFore, hintBack, false);
    }
}

std::size_t ResizePanel::VisualChildrenCount() const
{
    return children_.size();
}

VisualTreeNode* ResizePanel::GetVisualChild(std::size_t index) const
{
    return children_.at(index).get();
}

void ResizePanel::OnGotFocus()
{
    if (focusedIndex_ < children_.size())
    {
        ControlBase* current = children_[focusedIndex_].get();
        if (IsFocusablePanelContent(current))
        {
            FocusPanelContent(current);
            InvalidateVisual();
            return;
        }
    }

    for (std::size_t i = 0; i < children_.size(); ++i)
    {
        ControlBase* candidate = children_[i].get();
        if (IsFocusablePanelContent(candidate))
        {
            focusedIndex_ = i;
            FocusPanelContent(candidate);
            InvalidateVisual();
            return;
        }
    }

    InvalidateVisual();
}

void ResizePanel::OnLostFocus()
{
    focused_ = false;
    InvalidateVisual();
}

bool ResizePanel::MoveFocusNext(Direction direction, InputModifier modifiers)
{
    (void)modifiers;

    if (children_.empty())
        return false;

    if (direction == Direction::Next || direction == Direction::Down || direction == Direction::Right)
    {
        for (std::size_t i = focusedIndex_ + 1; i < children_.size(); ++i)
        {
            ControlBase* control = children_[i].get();
            if (IsFocusablePanelContent(control))
            {
                focusedIndex_ = i;
                FocusPanelContent(control);
                return true;
            }
        }

        return false;
    }

    if (direction == Direction::Previous || direction == Direction::Up || direction == Direction::Left)
    {
        if (focusedIndex_ == 0)
            return false;

        for (std::size_t i = focusedIndex_; i-- > 0;)
        {
            ControlBase* control = children_[i].get();
            if (IsFocusablePanelContent(control))
            {
                focusedIndex_ = i;
                FocusPanelContent(control);
                return true;
            }
        }

        return false;
    }

    return false;
}
