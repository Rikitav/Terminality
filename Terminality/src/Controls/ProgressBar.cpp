module terminality;

import std;

using namespace terminality;

Size ProgressBar::MeasureOverride(const Size& availableSize)
{
	int32_t width = availableSize.Width >= 0 ? std::max(availableSize.Width, 1) : 1;
	int32_t height = 1; // availableSize.Height >= 0 ? std::max(availableSize.Height, 1) : 1;
	return Size(width, height);
}

void ProgressBar::ArrangeOverride(const Rect& contentRect)
{
	// bleh U_U
	return;
}

void ProgressBar::RenderOverride(RenderContext& context)
{
	float minVal = Minimum.Get();
	float maxVal = Maximum.Get();
	float val = Value.Get();

	if (maxVal <= minVal)
		maxVal = minVal + 1.0f;

	Rect renderRect = context.ContextRect();
	float clampedVal = std::clamp(val, minVal, maxVal);
	float fraction = (clampedVal - minVal) / (maxVal - minVal);
	float exactWidth = fraction * renderRect.Width;

	int fullCells = static_cast<int>(exactWidth);
	int partialIndex = static_cast<int>((exactWidth - fullCells) * 2.0f);

	Color bar = BarColor.Get();
	Color track = TrackColor.Get();

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