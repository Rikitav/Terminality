
#include <string>
#include <cstdint>
#include <vector>
#include <functional>

#include <terminality/Controls/Spinner.hpp>

using namespace terminality;

std::vector<std::wstring> Spinner::Dots()
{
    return { L"⠋", L"⠙", L"⠹", L"⠸", L"⠼", L"⠴", L"⠦", L"⠧", L"⠇", L"⠏" };
}

std::vector<std::wstring> Spinner::Line()
{
    return { L"-", L"\\", L"|", L"/" };
}

std::vector<std::wstring> Spinner::Arrow()
{
    return { L">  ", L">> ", L">>>", L" >>", L"  >", L"   " };
}

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
            const std::size_t frameCount = Frames.Get().size();
            if (frameCount > 0)
                frame_ = (frame_ + 1) % static_cast<int>(frameCount);
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
    if (frames.empty())
        return;

    frame_ = std::clamp(frame_, 0, static_cast<int>(frames.size()) - 1);
    context.RenderText(Point::Zero, frames[frame_], ForegroundColor, BackgroundColor);
}
