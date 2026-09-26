#include "DiskList.hpp"

#include <algorithm>

using namespace terminality;

Size btop::DiskListControl::MeasureOverride(const Size& availableSize)
{
	int32_t width = availableSize.Width >= 0 ? std::max(availableSize.Width, 1) : 30;
	int32_t height = availableSize.Height >= 0 ? std::max(availableSize.Height, 1) : 1;
	return Size(width, height);
}

void btop::DiskListControl::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	int y = 0;

	for (const DiskInfo& disk : Disks)
	{
		if (y + 3 > rect.Height)
			break;

		// "C: System" ... "930.5 GiB"
		const std::wstring total = humanize(disk.total);
		context.RenderText(Point(0, y), disk.name, theme::mainFg, Color::BLACK);

		if (static_cast<int>(disk.name.size()) + static_cast<int>(total.size()) + 1 <= rect.Width)
		{
			context.RenderText(
				Point(rect.Width - static_cast<int>(total.size()), y),
				total,
				theme::graphText,
				Color::BLACK);
		}

		// Used meter.
		const int64_t pct = disk.total > 0
			? static_cast<int64_t>(disk.used * 100 / disk.total)
			: 0;
		
		DrawMeter(context, Rect(0, y + 1, rect.Width, 1), theme::used, pct);

		// "Used 45%  Free 512.3 GiB"
		std::wstring line = L"Used " + std::to_wstring(pct) + L"%  Free " + humanize(disk.free);
		if (static_cast<int>(line.size()) > rect.Width)
			line.resize(rect.Width);

		context.RenderText(Point(0, y + 2), line, theme::used.At(pct), Color::BLACK);
		y += 4;
	}
}
