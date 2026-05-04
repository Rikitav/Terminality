export module terminality:Grid;

import std;
import std.compat;
import :Geometry;
import :ControlBase;
import :RenderContext;

export namespace terminality
{
    enum class GridUnitType
    {
        Auto,
        Pixel,
        Star
    };

    struct GridLength
    {
        float Value;
        GridUnitType Type;

        GridLength(float value = 1.0f, GridUnitType type = GridUnitType::Star)
            : Value(value), Type(type) { }

        static GridLength Auto();
        static GridLength Pixel(int32_t pixels);
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
        size_t focusedIndex_ = 0;

        void EnsureGridDefinitions();

    public:
        Grid() = default;

        void AddRow(const RowDefinition& def);
        void AddColumn(const ColumnDefinition& def);
        void AddChild(std::unique_ptr<ControlBase> child, int32_t row = 0, int32_t column = 0, int32_t rowSpan = 1, int32_t colSpan = 1);

        bool MoveFocusNext(Direction direction, InputModifier modifiers) override;
        void OnGotFocus() override;
        void OnLostFocus() override;

    protected:
        Size MeasureOverride(const Size& availableSize) override;
        void ArrangeOverride(const Rect& contentRect) override;
        void RenderOverride(RenderContext& context) override;
    };
}