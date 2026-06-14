#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cstdint>

#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Framework/Property.hpp>
#include <terminality/Core/Geometry.hpp>
#include <terminality/Engine/RenderContext.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Core/Focus.hpp>
#include <terminality/Core/Layout.hpp>

namespace terminality
{
    class TabItem
    {
    public:
        std::string Header;
        std::unique_ptr<ControlBase> Content;
        
        TabItem(std::string header, std::unique_ptr<ControlBase> content) 
            : Header(std::move(header)), Content(std::move(content)) {}
    };

    class TabControl : public ControlBase
    {
    protected:
        std::vector<TabItem> tabs_;
        int32_t headerScrollOffset_ = 0;

    public:
        Property<TabControl, int> SelectedIndex { this, "SelectedIndex", 0, InvalidationKind::Arrange };

        TabControl() = default;

        void AddTab(const std::string& header, std::unique_ptr<ControlBase> content);
        void RemoveTab(int index);
        void ClearTabs();

        size_t GetTabCount() const;

        bool IsFocusable() const override { return true; }

        bool OnKeyDown(InputEvent input) override;

        std::size_t VisualChildrenCount() const override;
        VisualTreeNode* GetVisualChild(std::size_t index) const override;

        bool MoveFocusNext(Direction direction, InputModifier modifiers) override;
        void OnGotFocus() override;
        void OnLostFocus() override;
        void OnPropertyChanged(const char* propertyName) override;

    protected:
        Size MeasureOverride(const Size& availableSize) override;
        void ArrangeOverride(const Rect& finalRect) override;
        void RenderOverride(RenderContext& context) override;
    };
}
