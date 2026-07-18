#pragma once

#include <memory>
#include <algorithm>

#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Core/Layout.hpp>
#include <terminality/Engine/RenderContext.hpp>
#include <terminality/Framework/Property.hpp>
#include <terminality/Core/InputEvent.hpp>

namespace terminality
{
    class ScrollViewer : public ControlBase
    {
        void OnContentChanging(const std::unique_ptr<ControlBase>& oldContent);

    public:
        Property<ScrollViewer, std::unique_ptr<ControlBase>> Content { this, "Content", nullptr, InvalidationKind::Measure,
            [this](const std::unique_ptr<ControlBase>& old) { OnContentChanging(old); } };

        Property<ScrollViewer, int> ScrollX{ this, "ScrollX", 0, InvalidationKind::Arrange };
        Property<ScrollViewer, int> ScrollY{ this, "ScrollY", 0, InvalidationKind::Arrange };

        ScrollViewer();

        bool IsFocusable() const override
        {
            return true;
        }

        void OnPropertyChanged(const char* propertyName) override;

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
