export module terminality:Spinner;

import std;
import std.compat;
import :Focus;
import :Geometry;
import :ControlBase;
import :EventSignal;
import :DispatchTimer;

export namespace terminality
{
    class Spinner : public ControlBase
    {
        EventSignalConnection<float> tickConnection_;
        float accumulator_ = 0.0f;
        int frame_ = 0;

    public:
        PropertyDescriptor<Spinner, std::vector<std::wstring>> Frames = { this, "Frames", std::vector<std::wstring>{ L"-", L"\\", L"|", L"/" }, InvalidationKind::Visual };

        Spinner()
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

    protected:
        Size MeasureOverride(const Size& availableSize) override
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

            return Size(maxStr == nullptr ? 0 : maxStr->size(), 1);
        }

        void ArrangeOverride(const Rect& finalRect) override
        {
            return;
        }

        void RenderOverride(RenderContext& context) override
        {
            std::vector<std::wstring> frames = Frames.Get();
            context.RenderText(Point::Zero, frames.at(frame_), ForegroundColor, BackgroundColor);
        }
    };
}