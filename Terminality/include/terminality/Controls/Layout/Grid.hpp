#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <string_view>

#include <terminality/Core/InputEvent.hpp>
#include <terminality/Core/Focus.hpp>
#include <terminality/Core/Geometry.hpp>
#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Engine/RenderContext.hpp>

namespace terminality
{
    enum class GridUnitType
    {
        Auto,
        Cell,
        Star
    };

    struct GridLength
    {
        float Value;
        GridUnitType Type;

        GridLength(float value = 1.0f, GridUnitType type = GridUnitType::Star)
            : Value(value), Type(type) { }

        static GridLength Auto();
        static GridLength Cell(int32_t cells);
        static GridLength Star(float weight = 1.0f);
    };

    struct RowDefinition
    {
        GridLength Height = GridLength::Star();
        int32_t MinHeight = 0;
        int32_t MaxHeight = -1;

        int32_t ActualHeight = 0;
        int32_t OffsetY = 0;
    };

    struct ColumnDefinition
    {
        GridLength Width = GridLength::Star();
        int32_t MinWidth = 0;
        int32_t MaxWidth = -1;

        int32_t ActualWidth = 0;
        int32_t OffsetX = 0;
    };

    class Grid : public ControlBase
    {
        struct GridChild
        {
            std::unique_ptr<ControlBase> Control;
            int32_t Row;
            int32_t Column;
            int32_t RowSpan;
            int32_t ColumnSpan;
        };

        std::vector<RowDefinition> rowDefs_;
        std::vector<ColumnDefinition> colDefs_;
        std::vector<GridChild> children_;
        std::size_t focusedIndex_ = 0;

        void EnsureGridDefinitions();

        bool IsUniformMode() const;
        void GetUniformGrid(int32_t& columns, int32_t& rows) const;
        Size MeasureUniform(const Size& availableSize);
        void ArrangeUniform(const Rect& contentRect);

    public:
        Property<Grid, int> UniformColumns { this, "UniformColumns", 0, InvalidationKind::Measure };
        Property<Grid, int> UniformRows    { this, "UniformRows",    0, InvalidationKind::Measure };

        Grid() = default;

        void SetRowDefinitions(std::string_view definitions);
        void SetColumnDefinitions(std::string_view definitions);
        void AddRow(const RowDefinition& def);
        void AddColumn(const ColumnDefinition& def);
        void AddChild(int32_t row, int32_t column, int32_t rowSpan, int32_t colSpan, std::unique_ptr<ControlBase> child);
        void AddChild(int32_t row, int32_t column, std::unique_ptr<ControlBase> child);
        void AddChild(std::unique_ptr<ControlBase> child);

    protected:
        bool MoveFocusNext(Direction direction, InputModifier modifiers) override;
        void OnGotFocus() override;
        void OnLostFocus() override;

        Size MeasureOverride(const Size& availableSize) override;
        void ArrangeOverride(const Rect& contentRect) override;
        void RenderOverride(RenderContext& context) override;

        std::size_t VisualChildrenCount() const override;
        VisualTreeNode* GetVisualChild(std::size_t index) const override;
    };
}
