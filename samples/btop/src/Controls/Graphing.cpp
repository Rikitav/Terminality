#include "Graphing.hpp"

#include <algorithm>
#include <cmath>

using namespace terminality;

namespace
{
	// 25-entry glyph tables from btop's Symbols::graph_symbols (index = upper*5 + lower).
	const wchar_t BrailleUp[] = {
		L' ', L'\x2880', L'\x28A0', L'\x28B0', L'\x28B8',
		L'\x2840', L'\x28C0', L'\x28E0', L'\x28F0', L'\x28F8',
		L'\x2844', L'\x28C4', L'\x28E4', L'\x28F4', L'\x28FC',
		L'\x2846', L'\x28C6', L'\x28E6', L'\x28F6', L'\x28FE',
		L'\x2847', L'\x28C7', L'\x28E7', L'\x28F7', L'\x28FF'
	};

	const wchar_t BrailleDown[] = {
		L' ', L'\x2808', L'\x2818', L'\x2838', L'\x28B8',
		L'\x2801', L'\x2809', L'\x2819', L'\x2839', L'\x28B9',
		L'\x2803', L'\x280B', L'\x281B', L'\x283B', L'\x28BB',
		L'\x2807', L'\x280F', L'\x281F', L'\x283F', L'\x28BF',
		L'\x2847', L'\x284F', L'\x285F', L'\x287F', L'\x28FF'
	};

	const wchar_t BlockUp[] = {
		L' ', L'\x2597', L'\x2597', L'\x2590', L'\x2590',
		L'\x2596', L'\x2584', L'\x2584', L'\x259F', L'\x259F',
		L'\x2596', L'\x2584', L'\x2584', L'\x259F', L'\x259F',
		L'\x258C', L'\x2599', L'\x2599', L'\x2588', L'\x2588',
		L'\x258C', L'\x2599', L'\x2599', L'\x2588', L'\x2588'
	};

	const wchar_t BlockDown[] = {
		L' ', L'\x259D', L'\x259D', L'\x2590', L'\x2590',
		L'\x2598', L'\x2580', L'\x2580', L'\x259C', L'\x259C',
		L'\x2598', L'\x2580', L'\x2580', L'\x259C', L'\x259C',
		L'\x258C', L'\x259B', L'\x259B', L'\x2588', L'\x2588',
		L'\x258C', L'\x259B', L'\x259B', L'\x2588', L'\x2588'
	};

	const wchar_t* tableFor(btop::GraphSymbol symbol, bool invert)
	{
		if (symbol == btop::GraphSymbol::Block)
			return invert ? BlockDown : BlockUp;
		return invert ? BrailleDown : BrailleUp;
	}

	int64_t scaleValue(int64_t value, int64_t maxValue)
	{
		if (maxValue > 0)
			return std::clamp(value * 100 / maxValue, 0ll, 100ll);
		return std::clamp(value, 0ll, 100ll);
	}
}

void btop::DrawGraph(
	RenderContext& context,
	const Rect& rect,
	const std::deque<int64_t>& data,
	const Gradient& gradient,
	GraphSymbol symbol,
	bool invert,
	int64_t maxValue,
	bool noZero)
{
	if (rect.Width <= 0 || rect.Height <= 0)
		return;

	const Color bg = Color::BLACK;
	const auto* table = tableFor(symbol, invert);
	const int height = rect.Height;
	
	const float mod = (height == 1)
		? 0.3f
		: 0.1f;

	const int pairCount = static_cast<int>(data.size() / 2);
	const int cols = std::min(rect.Width, pairCount);
	if (cols <= 0)
		return;

	// Pairs are non-overlapping consecutive samples: {(0,1), (2,3), ...}
	// relative to the deque, matching btop's steady-state alignment.
	int dataOffset = static_cast<int>(data.size()) - cols * 2;
	if ((static_cast<int>(data.size()) - dataOffset) % 2 != 0)
		--dataOffset;

	for (int col = 0; col < cols; ++col)
	{
		const int64_t v0 = scaleValue(data[dataOffset + col * 2], maxValue);
		const int64_t v1 = scaleValue(data[dataOffset + col * 2 + 1], maxValue);

		for (int h = 0; h < height; ++h)
		{
			const int curHigh = height > 1
				? static_cast<int>(std::lround(100.0 * (height - h) / height))
				: 100;
			
			const int curLow = height > 1
				? static_cast<int>(std::lround(100.0 * (height - h - 1) / height))
				: 0;
			
			const int clampMin = (noZero && h == height - 1)
				? 1
				: 0;

			auto level = [&](int64_t value)
			{
				if (value >= curHigh)
					return 4;
				
				if (value <= curLow)
					return clampMin;
				
				return std::clamp(static_cast<int>(std::lround(
					(value - curLow) * 4.0 / (curHigh - curLow) + mod)),
					clampMin,
					4
				);
			};

			const int result0 = level(v0);
			const int result1 = level(v1);

			Color color;
			if (height == 1)
			{
				color = gradient.At(std::max(v0, v1));
			}
			else
			{
				const int row = h + 1; // 1-based from the top
				color = invert
					? gradient.At(row * 100 / height)
					: gradient.At(100 - ((row - 1) * 100 / height));
			}

			context.SetCell(
				rect.X + col,
				rect.Y + h,
				table[result0 * 5 + result1],
				color,
				bg
			);
		}
	}
}

Size btop::GraphControl::MeasureOverride(const Size& availableSize)
{
	int32_t width = availableSize.Width >= 0 ? std::max(availableSize.Width, 1) : 10;
	int32_t height = availableSize.Height >= 0 ? std::max(availableSize.Height, 1) : 4;
	return Size(width, height);
}

void btop::GraphControl::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	DrawGraph(context, Rect(0, 0, rect.Width, rect.Height), *Data, Gradient, Symbol, Invert, MaxValue, NoZero);
}

Size btop::MeterControl::MeasureOverride(const Size& availableSize)
{
	int32_t width = availableSize.Width >= 0 ? std::max(availableSize.Width, 1) : 10;
	return Size(width, 1);
}

void btop::MeterControl::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	DrawMeter(context, Rect(0, 0, rect.Width, rect.Height), Gradient, Value, Invert);
}

void btop::DrawMeter(
	RenderContext& context,
	const Rect& rect,
	const Gradient& gradient,
	int64_t value,
	bool invert)
{
	const int64_t clamped = std::clamp(value, 0ll, 100ll);

	for (int32_t i = 1; i <= rect.Width; ++i)
	{
		const int y = static_cast<int>(std::lround(i * 100.0 / rect.Width));

		if (clamped >= y)
		{
			context.SetCell(
				rect.X + i - 1,
				rect.Y,
				L'\x25A0',
				gradient.At(invert ? 100 - y : y),
				Color::BLACK
			);
		}
		else
		{
			context.SetCell(
				rect.X + i - 1,
				rect.Y,
				L'\x25A0',
				theme::meterBg,
				Color::BLACK
			);
		}
	}
}
