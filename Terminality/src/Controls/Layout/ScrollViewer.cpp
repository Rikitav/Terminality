module;

#include <algorithm>
#include <memory>

module terminality;

using namespace terminality;

Size ScrollViewer::MeasureOverride(const Size& availableSize)
{
    if (Content)
    {
        Size desired = Content->Measure(Size::Auto);
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

    if (Content != nullptr)
    {
        Size extent = Content->GetActualSize();

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

        Content->Arrange(contentRect);
    }
}

void ScrollViewer::RenderOverride(RenderContext& context)
{
    if (Content != nullptr)
    {
        RenderContext childContext = context.CreateInner(context.ContextRect());
        Content->Render(childContext);
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
    return Content ? Content->GetActualSize().Width : 0;
}

int ScrollViewer::GetExtentHeight() const
{
    return Content ? Content->GetActualSize().Height : 0;
}

int ScrollViewer::GetViewportWidth() const
{
    return GetArrangedRect().Width;
}

int ScrollViewer::GetViewportHeight() const
{
    return GetArrangedRect().Height;
}
