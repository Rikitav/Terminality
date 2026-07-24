
#include <algorithm>
#include <memory>
#include <cstring>

#include <terminality/Controls/ScrollViewer.hpp>

using namespace terminality;

ScrollViewer::ScrollViewer()
{
    FocusedForegroundColor = ForegroundColor;
    FocusedBackgroundColor = BackgroundColor;
}

void ScrollViewer::OnContentChanging(const std::unique_ptr<ControlBase>& oldContent)
{
    if (oldContent != nullptr)
        oldContent->SetParent(nullptr);
}

void ScrollViewer::OnPropertyChanged(const char* propertyName)
{
    if (std::strcmp(propertyName, "Content") == 0)
    {
        if (Content.Get() != nullptr)
            Content.Get()->SetParent(this);

        ScrollX = 0;
        ScrollY = 0;
        InvalidateMeasure();
        InvalidateVisual();
        return;
    }

    ControlBase::OnPropertyChanged(propertyName);
}

Size ScrollViewer::MeasureOverride(const Size& availableSize)
{
    if (Content.Get())
    {
        Size desired = Content.Get()->Measure(Size::Auto);
        return Size(
            std::min(availableSize.Width, desired.Width),
            std::min(availableSize.Height, desired.Height)
        );
    }

    return Size::Zero;
}

void ScrollViewer::ArrangeOverride(const Rect& finalRect)
{
    arrangedRect_ = finalRect;

    if (Content.Get() != nullptr)
    {
        Size extent = Content.Get()->GetActualSize();

        int maxScrollX = std::max(0, extent.Width - finalRect.Width);
        int maxScrollY = std::max(0, extent.Height - finalRect.Height);

        int cx = std::clamp(ScrollX.Get(), 0, maxScrollX);
        int cy = std::clamp(ScrollY.Get(), 0, maxScrollY);

        ScrollX = cx;
        ScrollY = cy;

        Rect contentRect(
            finalRect.X - cx,
            finalRect.Y - cy,
            extent.Width,
            extent.Height
        );

        Content.Get()->Arrange(contentRect);
    }
}

void ScrollViewer::RenderOverride(RenderContext& context)
{
    if (Content.Get() != nullptr)
    {
        RenderContext childContext = context.CreateInner(context.ContextRect());
        Content.Get()->Render(childContext);
    }

    int viewHeight = GetViewportHeight();
    int extentHeight = GetExtentHeight();
    
    if (extentHeight > viewHeight && viewHeight >= 3)
    {
        int maxScrollY = extentHeight - viewHeight;
        int cy = ScrollY.Get();
        
        float progress = (float)cy / maxScrollY;
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

bool ScrollViewer::OnKeyDown(InputEvent input)
{
    if (hasFlag(input.Modifier, InputModifier::LeftAlt) || hasFlag(input.Modifier, InputModifier::RightAlt))
        return false;

    int currentY = ScrollY.Get();
    if (input.Key == InputKey::UP)
    {
        ScrollY = std::max(0, currentY - 1);
        return true;
    }
    else if (input.Key == InputKey::DOWN)
    {
        int maxScrollY = std::max(0, GetExtentHeight() - GetViewportHeight());
        ScrollY = std::min(maxScrollY, currentY + 1);
        return true;
    }

    return false;
}

int ScrollViewer::GetExtentWidth() const
{
    return Content.Get() ? Content.Get()->GetActualSize().Width : 0;
}

int ScrollViewer::GetExtentHeight() const
{
    return Content.Get() ? Content.Get()->GetActualSize().Height : 0;
}

int ScrollViewer::GetViewportWidth() const
{
    return GetArrangedRect().Width;
}

int ScrollViewer::GetViewportHeight() const
{
    return GetArrangedRect().Height;
}
