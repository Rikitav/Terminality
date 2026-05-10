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
        EventConnection<float> tickConnection_;
        float accumulator_ = 0.0f;
        int frame_ = 0;

    public:
        Property<Spinner, std::vector<std::wstring>> Frames = { this, "Frames", std::vector<std::wstring>{ L"-", L"\\", L"|", L"/" }, InvalidationKind::Visual };

        Spinner();

    protected:
        Size MeasureOverride(const Size& availableSize) override;
        void ArrangeOverride(const Rect& finalRect) override;
        void RenderOverride(RenderContext& context) override;
    };
}