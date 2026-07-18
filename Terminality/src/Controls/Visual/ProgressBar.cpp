
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <chrono>

#include <terminality/Terminality.hpp>

using namespace terminality;

Size ProgressBar::MeasureOverride(const Size& availableSize)
{
	constexpr int32_t defaultWidth = 20;

	int32_t width = availableSize.Width >= 0 ? std::max(availableSize.Width, 1) : defaultWidth;
	int32_t height = 1;
	return Size(width, height);
}

void ProgressBar::ArrangeOverride(const Rect& contentRect)
{
	// bleh U_U
	return;
}

void ProgressBar::RenderOverride(RenderContext& context)
{
	Rect renderRect = context.ContextRect();
	Color bar = BarColor.Get();
	Color track = TrackColor.Get();

	if (IsIndeterminate)
	{
		int w = renderRect.Width;
		if (w <= 0)
			return;

		int block = std::max(1, w / 4);
		auto now = std::chrono::steady_clock::now().time_since_epoch();
		float t = std::chrono::duration<float>(now).count();
		float cycle = 2.0f;
		float phase = std::fmod(t, cycle) / cycle;
		int range = std::max(1, w - block);
		int start = static_cast<int>(phase * range + 0.5f);

		for (int y = 0; y < renderRect.Height; ++y)
		{
			for (int x = 0; x < w; ++x)
			{
				if (x >= start && x < start + block)
					context.SetCell(x, y, L'\x2588', bar, track);
				else
					context.SetCell(x, y, L' ', track, track);
			}
		}

		InvalidateVisual();
		return;
	}

	float minVal = Minimum.Get();
	float maxVal = Maximum.Get();
	float val = Value.Get();

	if (maxVal <= minVal)
		maxVal = minVal + 1.0f;

	float clampedVal = std::clamp(val, minVal, maxVal);
	float fraction = (clampedVal - minVal) / (maxVal - minVal);
	float exactWidth = fraction * renderRect.Width;

	int fullCells = static_cast<int>(exactWidth);
	int partialIndex = static_cast<int>((exactWidth - fullCells) * 2.0f);

	static const wchar_t partialBlocks[] = { L' ', L'\x258C' };
	for (int y = 0; y < renderRect.Height; ++y)
	{
		for (int x = 0; x < renderRect.Width; ++x)
		{
			if (x < fullCells)
			{
				context.SetCell(x, y, L'\x2588', bar, track);
			}
			else if (x == fullCells)
			{
				context.SetCell(x, y, partialBlocks[partialIndex], bar, track);
			}
			else
			{
				context.SetCell(x, y, L' ', track, track);
			}
		}
	}
}