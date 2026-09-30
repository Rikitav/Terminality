#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

#include <terminality/Core/Color.hpp>
#include <terminality/Core/Focus.hpp>
#include <terminality/Core/Layout.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Engine/DispatchTimer.hpp>
#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Framework/Event.hpp>

namespace terminality
{
    class ResizePanel : public ControlBase
    {
        struct Divider
        {
            InputKey RetractKey = InputKey::None;
            InputKey ExpandKey = InputKey::None;
            int32_t Offset = 0;                     // Cells from the base (even-share) position
            bool CustomKeys = false;                // Not overwritten when dividers are rebuilt
        };

        std::vector<std::unique_ptr<ControlBase>> children_;
        std::vector<Divider> dividers_;
        std::vector<int32_t> dividerPositions_;     // Resolved content-relative positions (set at arrange)
        std::size_t focusedIndex_ = 0;
        std::size_t pressedDivider_ = static_cast<std::size_t>(-1);
        std::chrono::steady_clock::time_point lastSplitterPress_{};
        bool shiftHeld_ = false;

        EventConnection<float> tickConnection_;

        void SyncDividers();
        bool IsVertical() const;
        bool IsShiftActive(const InputEvent& input) const;

    public:
        Property<ResizePanel, terminality::Orientation> Orientation { this, "Orientation", terminality::Orientation::Vertical, InvalidationKind::Measure };
        Property<ResizePanel, Color> SplitterColor                  { this, "SplitterColor", Color::DARK_GRAY, InvalidationKind::Visual };
        Property<ResizePanel, Color> FocusedSplitterColor           { this, "FocusedSplitterColor", Color::CYAN, InvalidationKind::Visual };
        Property<ResizePanel, bool> ShowHints                       { this, "ShowHints", true, InvalidationKind::Visual };

        Property<ResizePanel, Color> PressedForegroundColor  { this, "PressedForegroundColor", Color::BLACK, InvalidationKind::Visual };
        Property<ResizePanel, Color> PressedBackgroundColor  { this, "PressedBackgroundColor", Color::CYAN, InvalidationKind::Visual };

        Event<std::size_t> SplitterMoved;           // Fired with the divider index after every move

        ResizePanel();

        void AddChildControl(std::unique_ptr<ControlBase> child);

        template<typename T = ControlBase>
        inline T* AddChild(std::unique_ptr<T> child)
        {
            T* childPtr = child.get();
            this->AddChildControl(std::move(child));
            return childPtr;
        }

        std::unique_ptr<ControlBase> RemoveChildControl(ControlBase* child);
        std::unique_ptr<ControlBase> RemoveAt(std::size_t index);
        void Clear();

        std::size_t GetChildCount() const;
        std::size_t GetDividerCount() const;

        void SetSplitterKeys(std::size_t index, InputKey retractKey, InputKey expandKey);
        void MoveSplitter(std::size_t index, int32_t delta);
        int32_t GetSplitterOffset(std::size_t index) const;

        bool OnKeyDown(InputEvent input) override;
        bool OnKeyUp(InputEvent input) override;

    protected:
        bool MoveFocusNext(Direction direction, InputModifier modifiers) override;
        void OnGotFocus() override;
        void OnLostFocus() override;

        Size MeasureOverride(const Size& availableSize) override;
        void ArrangeOverride(const Rect& finalRect) override;
        void RenderOverride(RenderContext& context) override;

        std::size_t VisualChildrenCount() const override;
        VisualTreeNode* GetVisualChild(std::size_t index) const override;
    };
}
