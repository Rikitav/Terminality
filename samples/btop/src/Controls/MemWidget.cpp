#include "MemWidget.hpp"

#include "Theme.hpp"

using namespace terminality;

btop::MemWidget::MemWidget(History* history)
	: history_(history)
{
	HeaderText = L" MEM ";
	BorderColor = theme::memBox;
	ForegroundColor = theme::title;
	BackgroundColor = theme::mainBg;
	Padding = Thickness(1, 0, 1, 0);

	Content = init<Grid>([&](Grid* grid)
	{
		grid->SetColumnDefinitions("50*,50*");
		grid->HorizontalAlignment = HorizontalAlign::Stretch;
		grid->VerticalAlignment = VerticalAlign::Stretch;

		rows_ = grid->AddChild(0, 0, init<StatRowsControl>([](StatRowsControl*) {}));
		disks_ = grid->AddChild(0, 1, init<DiskListControl>([](DiskListControl* disks)
		{
			disks->Margin = Thickness(1, 1, 1, 1);
		}));
	});
}

void btop::MemWidget::SetGraphSymbol(GraphSymbol symbol)
{
	rows_->Symbol = symbol;
	rows_->InvalidateVisual();
}

void btop::MemWidget::Update(const Snapshot& snap)
{
	const MemData& mem = snap.mem;
	auto pct = [](uint64_t part, uint64_t total) -> int64_t
	{
		return total > 0 ? static_cast<int64_t>(part * 100 / total) : 0;
	};

	std::vector<StatRow> rows;
	auto addRow = [&](
		const wchar_t* label,
		const std::deque<int64_t>& history,
		uint64_t bytes,
		uint64_t total,
		const Gradient& gradient)
	{
		StatRow row;
		row.label = label;
		row.history = &history;
		row.percent = pct(bytes, total);
		row.bytes = bytes;
		row.gradient = gradient;
		rows.push_back(row);
	};

	addRow(L"Used:", history_->memUsed, mem.used, mem.total, theme::used);
	addRow(L"Available:", history_->memAvailable, mem.available, mem.total, theme::available);
	addRow(L"Cached:", history_->memCached, mem.cached, mem.total, theme::cached);
	addRow(L"Free:", history_->memFree, mem.free, mem.total, theme::free);

	if (mem.swapTotal > 0)
	{
		StatRow header;
		header.label = L"Swap:";
		header.isHeader = true;
		rows.push_back(header);
	
		addRow(L"Used:", history_->swapUsed, mem.swapUsed, mem.swapTotal, theme::used);
		addRow(L"Free:", history_->swapFree, mem.swapFree, mem.swapTotal, theme::free);
	}

	rows_->Rows = std::move(rows);
	rows_->InvalidateVisual();

	disks_->Disks = snap.disks;
	disks_->InvalidateVisual();
}
