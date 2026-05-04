module terminality;

import std;
import std.compat;

using namespace terminality;

GridLength GridLength::Auto()
{
    return { 0.0f, GridUnitType::Auto };
}

GridLength GridLength::Pixel(int32_t pixels)
{
    return { static_cast<float>(pixels), GridUnitType::Pixel };
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

void Grid::AddChild(std::unique_ptr<ControlBase> child, int32_t row, int32_t column, int32_t rowSpan, int32_t colSpan)
{
    if (!child)
        return;

    child->SetParent(this);
    if (!child->IsAttached())
        child->OnAttachedToTree();

    children_.push_back({ std::move(child), row, column, rowSpan, colSpan });
    InvalidateMeasure();
}

void Grid::EnsureGridDefinitions()
{
    // ������� �������� 1x1 �����, ���� ����������� �� ������
    if (rowDefs_.empty()) rowDefs_.push_back(RowDefinition{});
    if (colDefs_.empty()) colDefs_.push_back(ColumnDefinition{});
}

Size Grid::MeasureOverride(const Size& availableSize)
{
    EnsureGridDefinitions();
    for (auto& row : rowDefs_)
        row.ActualHeight = (row.Height.Type == GridUnitType::Pixel) ? static_cast<int32_t>(row.Height.Value) : 0;
    
    for (auto& col : colDefs_)
        col.ActualWidth = (col.Width.Type == GridUnitType::Pixel) ? static_cast<int32_t>(col.Width.Value) : 0;

    for (auto& childWrapper : children_)
    {
        int32_t rowIndex = std::clamp<int32_t>(childWrapper.Row, 0, static_cast<int32_t>(rowDefs_.size()) - 1);
        int32_t columnIndex = std::clamp<int32_t>(childWrapper.Column, 0, static_cast<int32_t>(colDefs_.size()) - 1);

        bool isRowAuto = rowDefs_[rowIndex].Height.Type == GridUnitType::Auto;
        bool isColAuto = colDefs_[columnIndex].Width.Type == GridUnitType::Auto;

        if (isRowAuto || isColAuto)
        {
            // �������� ���� ��������� ������, ����� ������ "�������" �������
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
        if (col.Width.Type == GridUnitType::Star) totalStarWidth += col.Width.Value;
        else fixedWidth += col.ActualWidth;
    }
    
    for (const auto& row : rowDefs_)
    {
        if (row.Height.Type == GridUnitType::Star) totalStarHeight += row.Height.Value;
        else fixedHeight += row.ActualHeight;
    }

    int32_t remainingWidth = std::max(0, availableSize.Width - fixedWidth);
    int32_t remainingHeight = std::max(0, availableSize.Height - fixedHeight);

    if (totalStarWidth > 0)
    {
        for (auto& col : colDefs_)
        {
            if (col.Width.Type == GridUnitType::Star)
                col.ActualWidth = static_cast<int32_t>((col.Width.Value / totalStarWidth) * remainingWidth);
        }
    }

    if (totalStarHeight > 0)
    {
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

    int32_t currentX = contentRect.X;
    for (auto& col : colDefs_)
    {
        col.OffsetX = currentX;
        currentX += col.ActualWidth;
    }

    int32_t currentY = contentRect.Y;
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

        Rect cellRect(colDefs_[columnIndex].OffsetX, rowDefs_[rowIndex].OffsetY, cellWidth, cellHeight);
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
    if (children_.empty())
    {
        InvalidateVisual();
        return;
    }

    if (focusedIndex_ >= children_.size())
        focusedIndex_ = 0;

    VisualTreeNode* focusedControl = children_[focusedIndex_].Control.get();
    PushFocus(focusedControl);
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

    switch (direction)
    {
        case Direction::Up:
        case Direction::Left:
        case Direction::Previous:
        {
            if (focusedIndex_ == 0)
                focusedIndex_ = children_.size() - 1;
            else
                focusedIndex_ -= 1;

            for (size_t i = focusedIndex_; i >= 0; i--)
            {
                ControlBase* control = children_[i].Control.get();
                if (control->IsFocusable() && control->IsTabStop())
                {
                    focusedIndex_ = i;
                    PushFocus(control);
                    return true;
                }
            }

            break;
        }

        case Direction::Down:
        case Direction::Right:
        case Direction::Next:
        {
            if (children_.size() - 1 == focusedIndex_)
                focusedIndex_ = 0;
            else
                focusedIndex_ += 1;

            for (size_t i = focusedIndex_; i < children_.size(); i++)
            {
                ControlBase* control = children_[i].Control.get();
                if (control->IsFocusable() && control->IsTabStop())
                {
                    focusedIndex_ = i;
                    PushFocus(control);
                    return true;
                }
            }

            break;
        }
    }

    return false;
}
