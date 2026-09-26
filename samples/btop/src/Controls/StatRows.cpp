#include "StatRows.hpp"

#include <algorithm>

using namespace terminality;

Size btop::StatRowsControl::MeasureOverride(const Size& availableSize)
{
	int32_t width = availableSize.Width >= 0
		? std::max(availableSize.Width, 1)
		: 30;
	
	int32_t height = availableSize.Height >= 0
		? std::max(availableSize.Height, 1)
		: 1;
	
	return Size(width, height);
}

void btop::StatRowsControl::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	const int labelW = 10;

	// Adaptive value width so the sparkline keeps room in narrow boxes.
	const int valueW = std::clamp((rect.Width - labelW - 2) / 2, 8, 17);
	const bool shorten = valueW < 15;
	const int graphX = labelW + 1;
	const int graphW = rect.Width - labelW - valueW - 2;

	for (size_t i = 0; i < Rows.size(); ++i)
	{
		const int y = static_cast<int>(i);
		if (y >= rect.Height)
			break;

		const StatRow& row = Rows[i];
		if (row.isHeader)
		{
			context.RenderText(
				Point(0, y),
				row.label,
				theme::title,
				Color::BLACK
			);

			continue;
		}

		context.RenderText(
			Point(0, y),
			row.label,
			theme::mainFg,
			Color::BLACK
		);

		if (graphW >= 6 && row.history != nullptr)
		{
			DrawGraph(
				context,
				Rect(graphX, y, graphW, 1),
				*row.history,
				row.gradient,
				Symbol,
				false, 0
			);
		}

		std::wstring value
			= humanize(row.bytes, shorten)
			+ L' '
			+ std::to_wstring(row.percent)
			+ L'%';
		
		if (static_cast<int>(value.size()) > valueW)
			value.resize(valueW);

		context.RenderText(
			Point(rect.Width - static_cast<int>(value.size()), y),
			value,
			row.gradient.At(row.percent),
			Color::BLACK
		);
	}
}
