module terminality;

import std;
import std.compat;

using namespace terminality;

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

void Grid::AddChild(int32_t row, int32_t column, int32_t rowSpan, int32_t colSpan, std::unique_ptr<ControlBase> child)
{
    if (!child)
        return;

    child->SetParent(this, layer_);
    if (!child->IsAttached())
        child->OnAttachedToTree();

    children_.push_back({ std::move(child), row, column, rowSpan, colSpan });
    InvalidateMeasure();
}

void Grid::AddChild(int32_t row, int32_t column, std::unique_ptr<ControlBase> child)
{
    if (!child)
        return;

    child->SetParent(this, layer_);
    if (!child->IsAttached())
        child->OnAttachedToTree();

    children_.push_back({ std::move(child), row, column, 1, 1 });
    InvalidateMeasure();
}

void Grid::EnsureGridDefinitions()
{
    if (rowDefs_.empty()) 
        rowDefs_.push_back(RowDefinition{});
    
    if (colDefs_.empty())
        colDefs_.push_back(ColumnDefinition{});
}

Size Grid::MeasureOverride(const Size& availableSize)
{
    EnsureGridDefinitions();

    bool widthIsInfinite = availableSize.Width < 0;
    bool heightIsInfinite = availableSize.Height < 0;

    for (auto& row : rowDefs_)
        row.ActualHeight = (row.Height.Type == GridUnitType::Cell) ? static_cast<int32_t>(row.Height.Value) : 0;

    for (auto& col : colDefs_)
        col.ActualWidth = (col.Width.Type == GridUnitType::Cell) ? static_cast<int32_t>(col.Width.Value) : 0;

    for (auto& childWrapper : children_)
    {
        int32_t rowIndex = std::clamp<int32_t>(childWrapper.Row, 0, static_cast<int32_t>(rowDefs_.size()) - 1);
        int32_t columnIndex = std::clamp<int32_t>(childWrapper.Column, 0, static_cast<int32_t>(colDefs_.size()) - 1);

        bool isRowAuto = rowDefs_[rowIndex].Height.Type == GridUnitType::Auto ||
            (rowDefs_[rowIndex].Height.Type == GridUnitType::Star && heightIsInfinite);

        bool isColAuto = colDefs_[columnIndex].Width.Type == GridUnitType::Auto ||
            (colDefs_[columnIndex].Width.Type == GridUnitType::Star && widthIsInfinite);

        if (isRowAuto || isColAuto)
        {
            Size childDesired = childWrapper.Control->Measure(availableSize);

            if (isColAuto)
                colDefs_[columnIndex].ActualWidth = std::max(colDefs_[columnIndex].ActualWidth, childDesired.Width);

            if (isRowAuto)
                rowDefs_[rowIndex].ActualHeight = std::max(rowDefs_[rowIndex].ActualHeight, childDesired.Height);
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
                col.ActualWidth = static_cast<int32_t>((col.Width.Value / totalStarWidth) * remainingWidth);
        }
    }

    if (totalStarHeight > 0 && !heightIsInfinite)
    {
        int32_t remainingHeight = std::max(0, availableSize.Height - fixedHeight);
        for (auto& row : rowDefs_)
        {
            if (row.Height.Type == GridUnitType::Star)
                row.ActualHeight = static_cast<int32_t>((row.Height.Value / totalStarHeight) * remainingHeight);
        }
    }

    for (auto& childWrapper : children_)
    {
        int32_t rowIndex = std::clamp<int32_t>(childWrapper.Row, 0, static_cast<int32_t>(rowDefs_.size()) - 1);
        int32_t columnIndex = std::clamp<int32_t>(childWrapper.Column, 0, static_cast<int32_t>(colDefs_.size()) - 1);

        int32_t cellWidth = 0;
        int32_t cellHeight = 0;

        for (int32_t i = 0; i < childWrapper.ColumnSpan && (columnIndex + i) < colDefs_.size(); ++i)
            cellWidth += colDefs_[columnIndex + i].ActualWidth;

        for (int32_t i = 0; i < childWrapper.RowSpan && (rowIndex + i) < rowDefs_.size(); ++i)
            cellHeight += rowDefs_[rowIndex + i].ActualHeight;

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
        int32_t rowIndex = std::clamp<int32_t>(childWrapper.Row, 0, static_cast<int32_t>(rowDefs_.size()) - 1);
        int32_t columnIndex = std::clamp<int32_t>(childWrapper.Column, 0, static_cast<int32_t>(colDefs_.size()) - 1);

        int32_t cellWidth = 0;
        int32_t cellHeight = 0;

        for (int32_t i = 0; i < childWrapper.ColumnSpan && (columnIndex + i) < colDefs_.size(); ++i)
            cellWidth += colDefs_[columnIndex + i].ActualWidth;

        for (int32_t i = 0; i < childWrapper.RowSpan && (rowIndex + i) < rowDefs_.size(); ++i)
            cellHeight += rowDefs_[rowIndex + i].ActualHeight;

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

    for (size_t i = 0; i < children_.size(); ++i)
    {
        VisualTreeNode* focusedControl = children_[i].Control.get();
        if (focusedControl->IsFocusable())
        {
            focusedIndex_ = i;
            PushFocus(focusedControl);
            break;
        }
    }

    InvalidateVisual();
    return;
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

    if (direction == Direction::Next || direction == Direction::Previous)
    {
        if (direction == Direction::Next)
        {
            for (size_t i = focusedIndex_ + 1; i < children_.size(); i++)
            {
                ControlBase* control = children_[i].Control.get();
                if (control->IsFocusable() && control->IsTabStop())
                {
                    focusedIndex_ = i;
                    PushFocus(control);
                    return true;
                }
            }
        }
        else if (direction == Direction::Previous)
        {
            if (focusedIndex_ == 0)
                return false;

            for (size_t i = focusedIndex_ - 1; i < children_.size(); i--)
            {
                ControlBase* control = children_[i].Control.get();
                if (control->IsFocusable() && control->IsTabStop())
                {
                    focusedIndex_ = i;
                    PushFocus(control);
                    return true;
                }
            }
        }
        else
        {
            return false;
        }
    }

    if (focusedIndex_ >= children_.size())
        return false;

    const auto& current = children_[focusedIndex_];

    int r1 = current.Row;
    int r2 = current.Row + std::max(1, current.RowSpan) - 1;
    int c1 = current.Column;
    int c2 = current.Column + std::max(1, current.ColumnSpan) - 1;

    size_t bestIndex = std::numeric_limits<size_t>::max();
    int minPrimary = std::numeric_limits<int>::max();
    int minSecondary = std::numeric_limits<int>::max();

    for (size_t i = 0; i < children_.size(); ++i)
    {
        if (i == focusedIndex_)
            continue;

        const auto& candidate = children_[i];
        ControlBase* ctrl = candidate.Control.get();

        if (!ctrl->IsFocusable() || !ctrl->IsTabStop())
            continue;

        int cr1 = candidate.Row;
        int cr2 = candidate.Row + std::max(1, candidate.RowSpan) - 1;
        int cc1 = candidate.Column;
        int cc2 = candidate.Column + std::max(1, candidate.ColumnSpan) - 1;

        int primaryDist = -1;
        int secondaryDist = 0;

        switch (direction)
        {
            case Direction::Right:
            {
                if (cc1 > c2)
                {
                    primaryDist = cc1 - c2;
                    secondaryDist = (cr2 >= r1 && cr1 <= r2) ? 0 : std::min(std::abs(cr1 - r2), std::abs(cr2 - r1));
                }

                break;
            }

            case Direction::Left:
            {
                if (cc2 < c1)
                {
                    primaryDist = c1 - cc2;
                    secondaryDist = (cr2 >= r1 && cr1 <= r2) ? 0 : std::min(std::abs(cr1 - r2), std::abs(cr2 - r1));
                }
            
                break;
            }

            case Direction::Down:
            {
                if (cr1 > r2)
                {
                    primaryDist = cr1 - r2;
                    secondaryDist = (cc2 >= c1 && cc1 <= c2) ? 0 : std::min(std::abs(cc1 - c2), std::abs(cc2 - c1));
                }
            
                break;
            }

            case Direction::Up:
            {
                if (cr2 < r1)
                {
                    primaryDist = r1 - cr2;
                    secondaryDist = (cc2 >= c1 && cc1 <= c2) ? 0 : std::min(std::abs(cc1 - c2), std::abs(cc2 - c1));
                }
            
                break;
            }

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

    if (bestIndex != std::numeric_limits<size_t>::max())
    {
        focusedIndex_ = bestIndex;
        PushFocus(children_[bestIndex].Control.get());
        return true;
    }

    return false;
}
