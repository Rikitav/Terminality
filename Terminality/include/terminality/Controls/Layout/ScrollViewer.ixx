module;

#include <memory>
#include <algorithm>

export module terminality:ScrollViewer;

import :ControlBase;
import :Layout;
import :RenderContext;
import :Property;
import :InputEvent;

export namespace terminality
{
    class ScrollViewer : public ControlBase
    {
    public:
        std::unique_ptr<ControlBase> Content;

        Property<ScrollViewer, int> ScrollX{ this, "ScrollX", 0, InvalidationKind::Arrange };
        Property<ScrollViewer, int> ScrollY{ this, "ScrollY", 0, InvalidationKind::Arrange };

        ScrollViewer() = default;

        bool IsFocusable() const override
        {
            return true;
        }

        Size MeasureOverride(const Size& availableSize) override;
        void ArrangeOverride(const Rect& finalRect) override;
        void RenderOverride(RenderContext& context) override;

        bool OnKeyDown(InputEvent input) override;

        int GetExtentWidth() const;
        int GetExtentHeight() const;
        int GetViewportWidth() const;
        int GetViewportHeight() const;
    };
}
