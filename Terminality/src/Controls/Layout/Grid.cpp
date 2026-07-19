
#include <cstdint>
#include <memory>
#include <algorithm>
#include <vector>
#include <limits>
#include <string>
#include <string_view>
#include <charconv>

#include <terminality/Terminality.hpp>

using namespace terminality;

// ------------------------------------------------------------------
// String helpers
// ------------------------------------------------------------------

static constexpr std::string_view Trim(std::string_view str)
{
    const auto first = str.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos)
        return {};

    const auto last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

static constexpr char ToLowerAscii(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c;
}

static constexpr bool EqualsIgnoreCase(std::string_view a, std::string_view b)
{
    if (a.size() != b.size())
        return false;

    for (std::size_t i = 0; i < a.size(); ++i)
        if (ToLowerAscii(a[i]) != ToLowerAscii(b[i]))
            return false;

    return true;
}

static std::vector<GridLength> ParseGridLengths(std::string_view definitions)
{
    std::vector<GridLength> result;
    if (definitions.empty())
        return result;

    size_t start = 0;
    while (start < definitions.size())
    {
        size_t end = definitions.find(',', start);
        std::string_view token = Trim(definitions.substr(start, end - start));

        if (!token.empty())
        {
            if (EqualsIgnoreCase(token, "Auto"))
            {
                result.push_back(GridLength::Auto());
            }
            else if (token.back() == '*')
            {
                if (token.size() == 1)
                {
                    result.push_back(GridLength::Star(1.0f));
                }
                else
                {
                    float value = 1.0f;
                    auto numPart = token.substr(0, token.size() - 1);
                    std::from_chars(numPart.data(), numPart.data() + numPart.size(), value);
                    result.push_back(GridLength::Star(value));
                }
            }
            else
            {
                int value = 0;
                std::from_chars(token.data(), token.data() + token.size(), value);
                result.push_back(GridLength::Cell(value));
            }
        }

        if (end == std::string_view::npos)
            break;

        start = end + 1;
    }

    return result;
}

// ------------------------------------------------------------------
// GridLength
// ------------------------------------------------------------------

GridLength GridLength::Auto()
{
    return { 0.0f, GridUnitType::Auto };
}

GridLength GridLength::Cell(int32_t cells)
{
    return { static_cast<float>(cells), GridUnitType::Cell };
}

GridLength GridLength::Star(float weight)
{
    return { weight, GridUnitType::Star };
}

// ------------------------------------------------------------------
// Grid
// ------------------------------------------------------------------

void Grid::SetRowDefinitions(std::string_view definitions)
{
    rowDefs_.clear();
    auto lengths = ParseGridLengths(definitions);

    for (const auto& length : lengths)
        AddRow(RowDefinition{ length });
}

void Grid::SetColumnDefinitions(std::string_view definitions)
{
    colDefs_.clear();
    auto lengths = ParseGridLengths(definitions);

    for (const auto& length : lengths)
        AddColumn(ColumnDefinition{ length });
}

void Grid::AddRow(const RowDefinition& def)
{
    rowDefs_.push_back(def);
    InvalidateMeasure();
}

void Grid::AddColumn(const ColumnDefinition& def)
{
    colDefs_.push_back(def);
    InvalidateMeasure();
}

static void AttachChild(ControlBase& child, Grid* parent)
{
    child.SetParent(parent);
    if (!child.IsAttached())
        child.OnAttachedToTree();
}

void Grid::AddChild(int32_t row, int32_t column, int32_t rowSpan, int32_t colSpan, std::unique_ptr<ControlBase> child)
{
    if (!child)
        return;

    AttachChild(*child, this);
    children_.push_back({ std::move(child), row, column, rowSpan, colSpan });
    InvalidateMeasure();
}

void Grid::AddChild(int32_t row, int32_t column, std::unique_ptr<ControlBase> child)
{
    AddChild(row, column, 1, 1, std::move(child));
}

void Grid::AddChild(std::unique_ptr<ControlBase> child)
{
    AddChild(0, 0, 1, 1, std::move(child));
}

bool Grid::IsUniformMode() const
{
    return UniformColumns.Get() > 0 || UniformRows.Get() > 0;
}

void Grid::GetUniformGrid(int32_t& columns, int32_t& rows) const
{
    int32_t count = static_cast<int32_t>(children_.size());
    columns = UniformColumns.Get();
    rows = UniformRows.Get();

    if (columns > 0 && rows > 0)
    {
        // Both explicit - keep as-is.
    }
    else if (columns > 0)
    {
        rows = count > 0 ? (count + columns - 1) / columns : 0;
    }
    else if (rows > 0)
    {
        columns = count > 0 ? (count + rows - 1) / rows : 0;
    }

    if (columns < 1) columns = 1;
    if (rows < 1) rows = 1;
}

Size Grid::MeasureUniform(const Size& availableSize)
{
    int32_t columns = 0, rows = 0;
    GetUniformGrid(columns, rows);

    if (children_.empty())
        return Size(0, 0);

    const bool widthIsInfinite = availableSize.Width < 0;
    const bool heightIsInfinite = availableSize.Height < 0;

    int32_t cellWidth = widthIsInfinite ? 0 : availableSize.Width / columns;
    int32_t cellHeight = heightIsInfinite ? 0 : availableSize.Height / rows;

    // Determine natural cell sizes when the grid size is unconstrained.
    for (const auto& childWrapper : children_)
    {
        Size probeSize(
            widthIsInfinite ? -1 : cellWidth,
            heightIsInfinite ? -1 : cellHeight);

        Size desired = childWrapper.Control->Measure(probeSize);

        if (widthIsInfinite)
            cellWidth = std::max(cellWidth, desired.Width);

        if (heightIsInfinite)
            cellHeight = std::max(cellHeight, desired.Height);
    }

    Size finalCell(cellWidth, cellHeight);
    for (const auto& childWrapper : children_)
        childWrapper.Control->Measure(finalCell);

    return Size(cellWidth * columns, cellHeight * rows);
}

void Grid::ArrangeUniform(const Rect& contentRect)
{
    int32_t columns = 0, rows = 0;
    GetUniformGrid(columns, rows);

    if (children_.empty())
        return;

    int32_t cellWidth = contentRect.Width / columns;
    int32_t cellHeight = contentRect.Height / rows;

    for (std::size_t i = 0; i < children_.size(); ++i)
    {
        int32_t col = static_cast<int32_t>(i) % columns;
        int32_t row = static_cast<int32_t>(i) / columns;

        Rect cellRect(
            contentRect.X + col * cellWidth,
            contentRect.Y + row * cellHeight,
            cellWidth,
            cellHeight);

        children_[i].Control->Arrange(cellRect);
    }
}

void Grid::EnsureGridDefinitions()
{
    if (rowDefs_.empty())
        rowDefs_.push_back(RowDefinition{});

    if (colDefs_.empty())
        colDefs_.push_back(ColumnDefinition{});
}

template <typename T>
static int32_t ClampIndex(int32_t index, const std::vector<T>& container)
{
    return std::clamp<int32_t>(index, 0, static_cast<int32_t>(container.size()) - 1);
}

static int32_t ClampGridDimension(int32_t value, int32_t min, int32_t max)
{
    int32_t lo = (min >= 0) ? min : value;
    int32_t hi = (max >= 0) ? max : value;
    if (hi < lo)
        hi = lo;

    return std::clamp(value, lo, hi);
}

template <typename T>
static int32_t SumSpan(const std::vector<T>& defs, int32_t startIndex, int32_t span,
                       int32_t T::* actualSize)
{
    int32_t sum = 0;
    for (int32_t i = 0; i < span && (startIndex + i) < static_cast<int32_t>(defs.size()); ++i)
        sum += defs[startIndex + i].*actualSize;

    return sum;
}

Size Grid::MeasureOverride(const Size& availableSize)
{
    if (IsUniformMode())
        return MeasureUniform(availableSize);

    EnsureGridDefinitions();

    const bool widthIsInfinite = availableSize.Width < 0;
    const bool heightIsInfinite = availableSize.Height < 0;

    for (auto& row : rowDefs_)
    {
        if (row.Height.Type == GridUnitType::Cell)
            row.ActualHeight = ClampGridDimension(static_cast<int32_t>(row.Height.Value), row.MinHeight, row.MaxHeight);
        else
            row.ActualHeight = 0;
    }

    for (auto& col : colDefs_)
    {
        if (col.Width.Type == GridUnitType::Cell)
            col.ActualWidth = ClampGridDimension(static_cast<int32_t>(col.Width.Value), col.MinWidth, col.MaxWidth);
        else
            col.ActualWidth = 0;
    }

    for (auto& childWrapper : children_)
    {
        int32_t rowIndex = ClampIndex(childWrapper.Row, rowDefs_);
        int32_t columnIndex = ClampIndex(childWrapper.Column, colDefs_);

        bool isRowAuto = rowDefs_[rowIndex].Height.Type == GridUnitType::Auto ||
            (rowDefs_[rowIndex].Height.Type == GridUnitType::Star && heightIsInfinite);

        bool isColAuto = colDefs_[columnIndex].Width.Type == GridUnitType::Auto ||
            (colDefs_[columnIndex].Width.Type == GridUnitType::Star && widthIsInfinite);

        if (isRowAuto || isColAuto)
        {
            Size childDesired = childWrapper.Control->Measure(availableSize);

            if (isColAuto)
            {
                int32_t clampedWidth = ClampGridDimension(childDesired.Width, colDefs_[columnIndex].MinWidth, colDefs_[columnIndex].MaxWidth);
                colDefs_[columnIndex].ActualWidth = std::max(colDefs_[columnIndex].ActualWidth, clampedWidth);
            }

            if (isRowAuto)
            {
                int32_t clampedHeight = ClampGridDimension(childDesired.Height, rowDefs_[rowIndex].MinHeight, rowDefs_[rowIndex].MaxHeight);
                rowDefs_[rowIndex].ActualHeight = std::max(rowDefs_[rowIndex].ActualHeight, clampedHeight);
            }
        }
    }

    int32_t fixedWidth = 0, fixedHeight = 0;
    float totalStarWidth = 0.0f, totalStarHeight = 0.0f;

    for (const auto& col : colDefs_)
    {
        if (col.Width.Type == GridUnitType::Star && !widthIsInfinite)
            totalStarWidth += col.Width.Value;
        else
            fixedWidth += col.ActualWidth;
    }

    for (const auto& row : rowDefs_)
    {
        if (row.Height.Type == GridUnitType::Star && !heightIsInfinite)
            totalStarHeight += row.Height.Value;
        else
            fixedHeight += row.ActualHeight;
    }

    if (totalStarWidth > 0 && !widthIsInfinite)
    {
        int32_t remainingWidth = std::max(0, availableSize.Width - fixedWidth);
        for (auto& col : colDefs_)
        {
            if (col.Width.Type == GridUnitType::Star)
            {
                int32_t starWidth = static_cast<int32_t>((col.Width.Value / totalStarWidth) * remainingWidth);
                col.ActualWidth = ClampGridDimension(starWidth, col.MinWidth, col.MaxWidth);
            }
        }
    }

    if (totalStarHeight > 0 && !heightIsInfinite)
    {
        int32_t remainingHeight = std::max(0, availableSize.Height - fixedHeight);
        for (auto& row : rowDefs_)
        {
            if (row.Height.Type == GridUnitType::Star)
            {
                int32_t starHeight = static_cast<int32_t>((row.Height.Value / totalStarHeight) * remainingHeight);
                row.ActualHeight = ClampGridDimension(starHeight, row.MinHeight, row.MaxHeight);
            }
        }
    }

    for (auto& childWrapper : children_)
    {
        int32_t rowIndex = ClampIndex(childWrapper.Row, rowDefs_);
        int32_t columnIndex = ClampIndex(childWrapper.Column, colDefs_);

        int32_t cellWidth = SumSpan(colDefs_, columnIndex, childWrapper.ColumnSpan, &ColumnDefinition::ActualWidth);
        int32_t cellHeight = SumSpan(rowDefs_, rowIndex, childWrapper.RowSpan, &RowDefinition::ActualHeight);

        childWrapper.Control->Measure(Size(cellWidth, cellHeight));
    }

    int32_t totalDesiredWidth = 0;
    int32_t totalDesiredHeight = 0;

    for (const auto& col : colDefs_)
        totalDesiredWidth += col.ActualWidth;

    for (const auto& row : rowDefs_)
        totalDesiredHeight += row.ActualHeight;

    return Size(totalDesiredWidth, totalDesiredHeight);
}

void Grid::ArrangeOverride(const Rect& contentRect)
{
    if (IsUniformMode())
    {
        ArrangeUniform(contentRect);
        return;
    }

    EnsureGridDefinitions();

    int32_t currentX = 0;
    for (auto& col : colDefs_)
    {
        col.OffsetX = currentX;
        currentX += col.ActualWidth;
    }

    int32_t currentY = 0;
    for (auto& row : rowDefs_)
    {
        row.OffsetY = currentY;
        currentY += row.ActualHeight;
    }

    for (auto& childWrapper : children_)
    {
        int32_t rowIndex = ClampIndex(childWrapper.Row, rowDefs_);
        int32_t columnIndex = ClampIndex(childWrapper.Column, colDefs_);

        int32_t cellWidth = SumSpan(colDefs_, columnIndex, childWrapper.ColumnSpan, &ColumnDefinition::ActualWidth);
        int32_t cellHeight = SumSpan(rowDefs_, rowIndex, childWrapper.RowSpan, &RowDefinition::ActualHeight);

        Rect cellRect(
            contentRect.X + colDefs_[columnIndex].OffsetX,
            contentRect.Y + rowDefs_[rowIndex].OffsetY,
            cellWidth,
            cellHeight);

        childWrapper.Control->Arrange(cellRect);
    }
}

void Grid::RenderOverride(RenderContext& context)
{
    for (const auto& childWrapper : children_)
    {
        Rect childRect = childWrapper.Control->GetArrangedRect();
        RenderContext childContext = context.CreateInner(childRect);
        childWrapper.Control->Render(childContext);
    }
}

size_t Grid::VisualChildrenCount() const
{
    return children_.size();
}

VisualTreeNode* Grid::GetVisualChild(std::size_t index) const
{
    return children_.at(index).Control.get();
}

void Grid::OnGotFocus()
{
    if (focusedIndex_ < children_.size())
    {
        VisualTreeNode* focusedControl = children_[focusedIndex_].Control.get();
        if (focusedControl->IsFocusable())
        {
            PushFocus(focusedControl);
            InvalidateVisual();
            return;
        }
    }

    for (std::size_t i = 0; i < children_.size(); ++i)
    {
        VisualTreeNode* focusedControl = children_[i].Control.get();
        if (focusedControl->IsFocusable())
        {
            focusedIndex_ = i;
            PushFocus(focusedControl);
            InvalidateVisual();
            return;
        }
    }

    InvalidateVisual();
}

void Grid::OnLostFocus()
{
    focused_ = false;
    InvalidateVisual();
}

bool Grid::MoveFocusNext(Direction direction, InputModifier modifiers)
{
    if (children_.empty())
        return false;

    if (direction == Direction::Next)
    {
        for (std::size_t i = focusedIndex_ + 1; i < children_.size(); ++i)
        {
            ControlBase* control = children_[i].Control.get();
            if (control->IsFocusable() && control->IsTabStop())
            {
                focusedIndex_ = i;
                PushFocus(control);
                return true;
            }
        }
        return false;
    }

    if (direction == Direction::Previous)
    {
        if (focusedIndex_ == 0)
            return false;

        for (std::size_t i = focusedIndex_; i-- > 0;)
        {
            ControlBase* control = children_[i].Control.get();
            if (control->IsFocusable() && control->IsTabStop())
            {
                focusedIndex_ = i;
                PushFocus(control);
                return true;
            }
        }
        return false;
    }

    if (focusedIndex_ >= children_.size())
        return false;

    const auto& current = children_[focusedIndex_];

    auto GetCellBounds = [](const GridChild& child) -> std::tuple<int, int, int, int>
    {
        int r1 = child.Row;
        int r2 = child.Row + std::max(1, child.RowSpan) - 1;
        int c1 = child.Column;
        int c2 = child.Column + std::max(1, child.ColumnSpan) - 1;
        return { r1, r2, c1, c2 };
    };

    auto [r1, r2, c1, c2] = GetCellBounds(current);

    size_t bestIndex = std::numeric_limits<std::size_t>::max();
    int minPrimary = std::numeric_limits<int>::max();
    int minSecondary = std::numeric_limits<int>::max();

    for (std::size_t i = 0; i < children_.size(); ++i)
    {
        if (i == focusedIndex_)
            continue;

        const auto& candidate = children_[i];
        ControlBase* ctrl = candidate.Control.get();

        if (!ctrl->IsFocusable() || !ctrl->IsTabStop())
            continue;

        auto [cr1, cr2, cc1, cc2] = GetCellBounds(candidate);

        int primaryDist = -1;
        int secondaryDist = 0;

        switch (direction)
        {
            case Direction::Right:
                if (cc1 > c2)
                {
                    primaryDist = cc1 - c2;
                    secondaryDist = (cr2 >= r1 && cr1 <= r2) ? 0 : std::min(std::abs(cr1 - r2), std::abs(cr2 - r1));
                }
                break;

            case Direction::Left:
                if (cc2 < c1)
                {
                    primaryDist = c1 - cc2;
                    secondaryDist = (cr2 >= r1 && cr1 <= r2) ? 0 : std::min(std::abs(cr1 - r2), std::abs(cr2 - r1));
                }
                break;

            case Direction::Down:
                if (cr1 > r2)
                {
                    primaryDist = cr1 - r2;
                    secondaryDist = (cc2 >= c1 && cc1 <= c2) ? 0 : std::min(std::abs(cc1 - c2), std::abs(cc2 - c1));
                }
                break;

            case Direction::Up:
                if (cr2 < r1)
                {
                    primaryDist = r1 - cr2;
                    secondaryDist = (cc2 >= c1 && cc1 <= c2) ? 0 : std::min(std::abs(cc1 - c2), std::abs(cc2 - c1));
                }
                break;

            default:
                break;
        }

        if (primaryDist > 0)
        {
            if (primaryDist < minPrimary || (primaryDist == minPrimary && secondaryDist < minSecondary))
            {
                minPrimary = primaryDist;
                minSecondary = secondaryDist;
                bestIndex = i;
            }
        }
    }

    if (bestIndex != std::numeric_limits<std::size_t>::max())
    {
        focusedIndex_ = bestIndex;
        PushFocus(children_[bestIndex].Control.get());
        return true;
    }

    return false;
}
