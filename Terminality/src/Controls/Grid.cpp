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

Size Grid::Measure(const Size& availableSize)
{
    EnsureGridDefinitions();

    // �������� ����������� ������ Grid (MinSize/MaxSize)
    Size constrainedSize = ControlBase::Measure(availableSize);

    // 1. �������������� ������������� ������� (Pixel) � ���������� ���������
    for (auto& row : rowDefs_) row.ActualHeight = (row.Height.Type == GridUnitType::Pixel) ? static_cast<int32_t>(row.Height.Value) : 0;
    for (auto& col : colDefs_) col.ActualWidth = (col.Width.Type == GridUnitType::Pixel) ? static_cast<int32_t>(col.Width.Value) : 0;

    // 2. ���� 1: ��������� ������� Auto-�����
    // �������� ������ ��� �����, ��� ������� ������ �� �����
    for (auto& childWrapper : children_) {
        int32_t r = std::clamp<int32_t>(childWrapper.Row, 0, static_cast<int32_t>(rowDefs_.size()) - 1);
        int32_t c = std::clamp<int32_t>(childWrapper.Column, 0, static_cast<int32_t>(colDefs_.size()) - 1);

        bool isRowAuto = rowDefs_[r].Height.Type == GridUnitType::Auto;
        bool isColAuto = colDefs_[c].Width.Type == GridUnitType::Auto;

        if (isRowAuto || isColAuto) {
            // �������� ���� ��������� ������, ����� ������ "�������" �������
            Size childDesired = childWrapper.Control->Measure(constrainedSize);

            if (isColAuto) {
                colDefs_[c].ActualWidth = std::max(colDefs_[c].ActualWidth, childDesired.Width);
            }
            if (isRowAuto) {
                rowDefs_[r].ActualHeight = std::max(rowDefs_[r].ActualHeight, childDesired.Height);
            }
        }
    }

    // 3. �������, ������� ����� ������ Pixel � Auto, � ������� Star-����� � ��� ����
    int32_t fixedWidth = 0, fixedHeight = 0;
    float totalStarWidth = 0.0f, totalStarHeight = 0.0f;

    for (const auto& col : colDefs_) {
        if (col.Width.Type == GridUnitType::Star) totalStarWidth += col.Width.Value;
        else fixedWidth += col.ActualWidth;
    }
    for (const auto& row : rowDefs_) {
        if (row.Height.Type == GridUnitType::Star) totalStarHeight += row.Height.Value;
        else fixedHeight += row.ActualHeight;
    }

    // 4. ��������� ������� Star-�����, ����������� ������� ���������� �����
    int32_t remainingWidth = std::max(0, constrainedSize.Width - fixedWidth);
    int32_t remainingHeight = std::max(0, constrainedSize.Height - fixedHeight);

    if (totalStarWidth > 0) {
        for (auto& col : colDefs_) {
            if (col.Width.Type == GridUnitType::Star)
                col.ActualWidth = static_cast<int32_t>((col.Width.Value / totalStarWidth) * remainingWidth);
        }
    }

    if (totalStarHeight > 0) {
        for (auto& row : rowDefs_) {
            if (row.Height.Type == GridUnitType::Star)
                row.ActualHeight = static_cast<int32_t>((row.Height.Value / totalStarHeight) * remainingHeight);
        }
    }

    // ==========================================
    // 5. ���� 2: ��������� ������ (�����������)
    // ==========================================
    // ������ ����� ������������. �� ������� �������� ������� ������� 
    // � ���������� ��������� ��� ������, ����� �� ������� ���� actualSize_.
    for (auto& childWrapper : children_) {
        int32_t r = std::clamp<int32_t>(childWrapper.Row, 0, static_cast<int32_t>(rowDefs_.size()) - 1);
        int32_t c = std::clamp<int32_t>(childWrapper.Column, 0, static_cast<int32_t>(colDefs_.size()) - 1);

        int32_t cellWidth = 0;
        int32_t cellHeight = 0;

        // ��������� Span (����������� �����)
        for (int32_t i = 0; i < childWrapper.ColumnSpan && (c + i) < colDefs_.size(); ++i) {
            cellWidth += colDefs_[c + i].ActualWidth;
        }
        for (int32_t i = 0; i < childWrapper.RowSpan && (r + i) < rowDefs_.size(); ++i) {
            cellHeight += rowDefs_[r + i].ActualHeight;
        }

        // ���������� �������� ������� "�������" ��������� ������ ������
        childWrapper.Control->Measure(Size(cellWidth, cellHeight));
    }

    // 6. ������������ �������� ������, ������� Grid �������� � ������ ��������
    int32_t totalDesiredWidth = 0;
    int32_t totalDesiredHeight = 0;
    for (const auto& col : colDefs_) totalDesiredWidth += col.ActualWidth;
    for (const auto& row : rowDefs_) totalDesiredHeight += row.ActualHeight;

    measureDirty_ = false;

    // ��������� actualSize_ ������ ����� � ���������� ���
    actualSize_ = Size(totalDesiredWidth, totalDesiredHeight);
    return actualSize_;
}

void Grid::Arrange(const Rect& contentRect)
{
    ControlBase::Arrange(contentRect);
	Rect arrangedRect = GetArrangedRect();

    EnsureGridDefinitions();

    // 1. ��������� ���������� �������� (Offsets) ��� ������� � �����
    int32_t currentX = arrangedRect.X;
    for (auto& col : colDefs_)
    {
        col.OffsetX = currentX;
        currentX += col.ActualWidth;
    }

    int32_t currentY = arrangedRect.Y;
    for (auto& row : rowDefs_)
    {
        row.OffsetY = currentY;
        currentY += row.ActualHeight;
    }

    // 2. ����������� �������� �������� � �� �������
    for (auto& childWrapper : children_)
    {
        int32_t r = std::clamp<int32_t>(childWrapper.Row, 0, static_cast<int32_t>(rowDefs_.size()) - 1);
        int32_t c = std::clamp<int32_t>(childWrapper.Column, 0, static_cast<int32_t>(colDefs_.size()) - 1);

        // ��������� Span (������������ �� ��������� �����)
        int32_t cellWidth = 0;
        int32_t cellHeight = 0;

        for (int32_t i = 0; i < childWrapper.ColumnSpan && (c + i) < colDefs_.size(); ++i)
            cellWidth += colDefs_[c + i].ActualWidth;

        for (int32_t i = 0; i < childWrapper.RowSpan && (r + i) < rowDefs_.size(); ++i)
            cellHeight += rowDefs_[r + i].ActualHeight;

        Rect cellRect(colDefs_[c].OffsetX, rowDefs_[r].OffsetY, cellWidth, cellHeight);
        childWrapper.Control->Arrange(cellRect);
    }

    arrangedRect_ = arrangedRect;
    arrangeDirty_ = false;
}

void Grid::Render(RenderContext& context)
{
    for (const auto& childWrapper : children_)
    {
        Rect childRect = childWrapper.Control->GetArrangedRect();
        RenderContext childContext = context.CreateInner(childRect);
        childWrapper.Control->Render(childContext);
    }

    visualDirty_ = false;
}

// ���������� ��������� ������ (�������� �� ������� ����������)
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
    FocusManager::Current().SetFocused(focusedControl);
    InvalidateVisual();
}

void Grid::OnLostFocus()
{
    focused_ = false;
    InvalidateVisual();
}

bool Grid::MoveFocusNext(NavigationDirection direction)
{
    if (children_.empty())
        return false;

    switch (direction)
    {
        case NavigationDirection::Up:
        case NavigationDirection::Left:
        case NavigationDirection::Previous:
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
                    FocusManager::Current().SetFocused(control);
                    return true;
                }
            }

            break;
        }

        case NavigationDirection::Down:
        case NavigationDirection::Right:
        case NavigationDirection::Next:
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
                    FocusManager::Current().SetFocused(control);
                    return true;
                }
            }

            break;
        }
    }

    return false;
}
