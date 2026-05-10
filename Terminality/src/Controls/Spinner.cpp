module terminality;

import std;
import std.compat;

using namespace terminality;

Spinner::Spinner()
{
    SetFocusable(false);
    MinSize = Size(1, 1);
    MaxSize = Size(1, 1);

    tickConnection_ = DispatchTimer::Current().TickEvent.Connect([this](float dt)
    {
        accumulator_ += dt;
        if (accumulator_ >= 0.1f)
        {
            accumulator_ -= 0.1f;
            frame_ = (frame_ + 1) % 4;
            InvalidateVisual();
        }
    });
}

Size Spinner::MeasureOverride(const Size& availableSize)
{
    const std::wstring* maxStr = nullptr;
    for (auto& frame : Frames.Get())
    {
        if (maxStr == nullptr)
        {
            maxStr = &frame;
            continue;
        }

        if (frame.size() > maxStr->size())
        {
            maxStr = &frame;
            continue;
        }
    }

    return Size(maxStr == nullptr ? 0 : static_cast<int32_t>(maxStr->size()), 1);
}

void Spinner::ArrangeOverride(const Rect& finalRect)
{
    return;
}

void Spinner::RenderOverride(RenderContext& context)
{
    std::vector<std::wstring> frames = Frames.Get();
    context.RenderText(Point::Zero, frames.at(frame_), ForegroundColor, BackgroundColor);
}
