#include "CoreGrid.hpp"

#include <algorithm>

using namespace terminality;

namespace
{
	constexpr int LabelWidth = 3;  // "C0 "
	constexpr int PctWidth = 4;    // "100%"
	constexpr int Spacing = 1;
}

Size btop::CoreGridControl::MeasureOverride(const Size& availableSize)
{
	int32_t width = availableSize.Width >= 0 ? std::max(availableSize.Width, 1) : 40;
	int32_t height = availableSize.Height >= 0 ? std::max(availableSize.Height, 1) : 8;
	return Size(width, height);
}

void btop::CoreGridControl::RenderOverride(RenderContext& context)
{
	if (Cores == nullptr)
		return;

	const Rect rect = context.ContextRect();
	const size_t count = Cores->size();
	if (rect.Width < 12 || rect.Height < 1 || count == 0)
		return;

	const int rowsFit = rect.Height;
	const int colsNeeded = static_cast<int>((count + rowsFit - 1) / rowsFit);
	const int colW = std::max(12, rect.Width / colsNeeded);
	const int sparkW = std::max(4, colW - LabelWidth - PctWidth - Spacing * 2);

	for (size_t i = 0; i < count; ++i)
	{
		const int col = static_cast<int>(i) / rowsFit;
		const int row = static_cast<int>(i) % rowsFit;
		const int x = col * colW;
		const int y = row;
		if (x + colW > rect.Width)
			continue;

		const auto& history = (*Cores)[i];
		const int64_t pct = history.empty() ? 0 : history.back();

		wchar_t label[8] = {};
		swprintf(label, 8, L"C%zu", i);
		context.RenderText(Point(x, y), label, theme::hiFg, Color::BLACK);

		DrawGraph(context, Rect(x + LabelWidth, y, sparkW, 1), history, theme::cpu, Symbol, false, 0);

		wchar_t pctText[8] = {};
		swprintf(pctText, 8, L"%3lld%%", pct);
		context.RenderText(Point(x + LabelWidth + sparkW + Spacing, y), pctText, theme::cpu.At(pct), Color::BLACK);
	}
}
