
#include <cstdint>
#include <algorithm>
#include <string>
#include <vector>

#include <terminality/Terminality.hpp>

using namespace terminality;

static std::wstring ToWString(const std::string& text)
{
	return std::wstring(text.begin(), text.end());
}

void DataGrid::AddColumn(const std::wstring& header, int32_t width)
{
	columns_.push_back(DataGridColumn{ header, std::max(1, width) });
	InvalidateMeasure();
}

void DataGrid::AddColumn(const std::string& header, int32_t width)
{
	AddColumn(ToWString(header), width);
}

void DataGrid::ClearColumns()
{
	columns_.clear();
	InvalidateMeasure();
}

int32_t DataGrid::TotalColumnWidth() const
{
	int32_t total = 0;
	for (const auto& col : columns_)
		total += col.Width;

	if (!columns_.empty())
		total += static_cast<int32_t>(columns_.size() - 1);

	return total;
}

void DataGrid::ScrollToSelection()
{
	const Rect rect = GetArrangedRect();
	int32_t viewRows = std::max(1, rect.Height - 2);
	int32_t rows = static_cast<int32_t>(ItemsSource.Get().size());

	if (SelectedIndex < 0 && rows > 0)
		SelectedIndex = 0;

	int32_t selected = SelectedIndex.Get();
	if (selected < 0)
		return;

	if (selected < firstVisibleRow_)
		firstVisibleRow_ = selected;

	if (selected >= firstVisibleRow_ + viewRows)
		firstVisibleRow_ = selected - viewRows + 1;

	firstVisibleRow_ = std::clamp(firstVisibleRow_, 0, std::max(0, rows - viewRows));
}

Size DataGrid::MeasureOverride(const Size& availableSize)
{
	int32_t totalWidth = TotalColumnWidth();
	int32_t rows = static_cast<int32_t>(ItemsSource.Get().size());
	int32_t desiredHeight = rows + 2;

	int32_t width = availableSize.Width >= 0 ? std::max(availableSize.Width, totalWidth) : totalWidth;
	int32_t height = availableSize.Height >= 0 ? std::min(availableSize.Height, desiredHeight) : desiredHeight;

	return Size(width, height);
}

void DataGrid::ArrangeOverride(const Rect& /*contentRect*/)
{
	ScrollToSelection();
}

bool DataGrid::OnKeyDown(InputEvent input)
{
	if (!IsEnabled)
		return ControlBase::OnKeyDown(input);

	const auto& items = ItemsSource.Get();
	int32_t rows = static_cast<int32_t>(items.size());
	int32_t selected = SelectedIndex.Get();

	auto changeSelection = [&](int32_t newIndex)
	{
		newIndex = std::clamp(newIndex, -1, rows - 1);
		if (newIndex < 0 && rows > 0)
			newIndex = 0;

		if (newIndex != selected)
		{
			SelectedIndex = newIndex;
			ScrollToSelection();
			SelectionChanged.Emit(newIndex);
		}
	};

	Rect rect = GetArrangedRect();
	int32_t viewRows = std::max(1, rect.Height - 2);
	int32_t maxScroll = std::max(0, TotalColumnWidth() - rect.Width);

	switch (input.Key)
	{
		case InputKey::UP:
		{
			changeSelection(selected - 1);
			return true;
		}

		case InputKey::DOWN:
		{
			changeSelection(selected + 1);
			return true;
		}

		case InputKey::PRIOR:
		{
			changeSelection(selected - viewRows);
			return true;
		}

		case InputKey::NEXT:
		{
			changeSelection(selected + viewRows);
			return true;
		}

		case InputKey::HOME:
		{
			changeSelection(0);
			return true;
		}

		case InputKey::END:
		{
			changeSelection(rows - 1);
			return true;
		}

		case InputKey::LEFT:
		{
			scrollOffset_ = std::max(0, scrollOffset_ - 3);
			InvalidateVisual();
			return true;
		}

		case InputKey::RIGHT:
		{
			scrollOffset_ = std::min(maxScroll, scrollOffset_ + 3);
			InvalidateVisual();
			return true;
		}

		case InputKey::RETURN:
		case InputKey::SPACE:
		{
			if (selected >= 0 && selected < rows)
				SelectionChanged.Emit(selected);
			return true;
		}
	}

	return ControlBase::OnKeyDown(input);
}

void DataGrid::RenderOverride(RenderContext& context)
{
	const Rect bounds = context.ContextRect();
	if (bounds.Width <= 0 || bounds.Height <= 0)
		return;

	const int32_t totalWidth = TotalColumnWidth();
	const int32_t viewWidth = bounds.Width;
	scrollOffset_ = std::clamp(scrollOffset_, 0, std::max(0, totalWidth - viewWidth));

	const Color headerFg = HeaderForegroundColor.Get();
	const Color headerBg = HeaderBackgroundColor.Get();
	const Color lineFg = Color::DARK_GRAY;
	const Color normalFg = GetEffectiveForegroundColor();
	const Color normalBg = GetEffectiveBackgroundColor();
	const Color selectedFg = SelectedForegroundColor.Get();
	const Color selectedBg = SelectedBackgroundColor.Get();

	// Header row
	int32_t x = 0;
	for (std::size_t ci = 0; ci < columns_.size(); ++ci)
	{
		const auto& col = columns_[ci];
		int32_t colX = x - scrollOffset_;

		if (colX + col.Width > 0 && colX < viewWidth)
		{
			std::wstring text = col.Header;
			if (static_cast<int32_t>(text.length()) > col.Width)
				text = text.substr(0, col.Width);

			context.RenderText(Point(colX, 0), text, headerFg, headerBg);

			for (int32_t fill = static_cast<int32_t>(text.length()); fill < col.Width; ++fill)
				context.SetCell(colX + fill, 0, L' ', headerFg, headerBg);
		}

		x += col.Width;
		if (ci + 1 < columns_.size())
		{
			int32_t sepX = x - scrollOffset_;
			if (ShowGridLines && sepX >= 0 && sepX < viewWidth)
				context.SetCell(sepX, 0, L'\u2502', lineFg, headerBg);

			x += 1;
		}
	}

	// Horizontal line under the header.
	if (bounds.Height > 1)
	{
		for (int cx = 0; cx < viewWidth; ++cx)
			context.SetCell(cx, 1, L'\u2500', lineFg, normalBg);
	}

	// Data rows
	if (bounds.Height <= 2)
		return;

	const auto& items = ItemsSource.Get();
	int32_t selected = SelectedIndex.Get();
	int32_t viewRows = bounds.Height - 2;

	for (int32_t ri = 0; ri < viewRows; ++ri)
	{
		int32_t rowIndex = firstVisibleRow_ + ri;
		if (rowIndex < 0 || rowIndex >= static_cast<int32_t>(items.size()))
			break;

		int32_t rowY = 2 + ri;
		bool isSelected = (rowIndex == selected);
		Color fg = isSelected ? selectedFg : normalFg;
		Color bg = isSelected ? selectedBg : normalBg;

		x = 0;
		for (std::size_t ci = 0; ci < columns_.size(); ++ci)
		{
			const auto& col = columns_[ci];
			int32_t colX = x - scrollOffset_;

			std::wstring text;
			if (ci < items[rowIndex].size())
				text = items[rowIndex][ci];

			if (static_cast<int32_t>(text.length()) > col.Width)
				text = text.substr(0, col.Width);

			if (colX + col.Width > 0 && colX < viewWidth)
			{
				context.RenderText(Point(colX, rowY), text, fg, bg);

				for (int32_t fill = static_cast<int32_t>(text.length()); fill < col.Width; ++fill)
					context.SetCell(colX + fill, rowY, L' ', fg, bg);
			}

			x += col.Width;
			if (ci + 1 < columns_.size())
			{
				int32_t sepX = x - scrollOffset_;
				if (ShowGridLines && sepX >= 0 && sepX < viewWidth)
					context.SetCell(sepX, rowY, L'\u2502', lineFg, bg);

				x += 1;
			}
		}
	}
}
