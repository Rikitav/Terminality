#include "CpuWidget.hpp"

#include <cstdio>

#include "Theme.hpp"

using namespace terminality;

namespace
{
	std::unique_ptr<Label> makeLabel(const std::wstring& text, btop::Color color = btop::theme::mainFg)
	{
		return init<Label>([&](Label* label)
		{
			label->Text = text;
			label->ForegroundColor = color;
			label->BackgroundColor = btop::theme::mainBg;
		});
	}
}

btop::CpuWidget::CpuWidget(History* history)
{
	HeaderText = L" CPU ";
	BorderColor = theme::cpuBox;
	ForegroundColor = theme::title;
	BackgroundColor = theme::mainBg;
	Padding = Thickness(1, 0, 1, 0);

	auto left = init<Grid>([&](Grid* grid)
	{
		grid->SetRowDefinitions("50*,50*");
		grid->HorizontalAlignment = HorizontalAlign::Stretch;
		grid->VerticalAlignment = VerticalAlign::Stretch;
		
		graphUpper_ = grid->AddChild(0, 0, init<GraphControl>([history](GraphControl* graph)
		{
			graph->Data = &history->cpuTotal;
			graph->Gradient = theme::cpu;
		}));
		
		graphLower_ = grid->AddChild(1, 0, init<GraphControl>([history](GraphControl* graph)
		{
			graph->Data = &history->cpuTotal;
			graph->Gradient = theme::cpu;
			graph->Invert = true;
		}));
	});

	auto right = init<Grid>([&](Grid* grid)
	{
		grid->SetRowDefinitions("Auto,Auto,*");
		grid->HorizontalAlignment = HorizontalAlign::Stretch;
		grid->VerticalAlignment = VerticalAlign::Stretch;
		
		grid->AddChild(0, 0, init<Grid>([&](Grid* grid)
		{
			grid->SetRowDefinitions("Auto");
			grid->SetColumnDefinitions("Auto,*,Auto");
			grid->HorizontalAlignment = HorizontalAlign::Stretch;

			grid->AddChild(0, 0, makeLabel(L"CPU", theme::title));
			meter_ = grid->AddChild(0, 1, init<MeterControl>([](MeterControl* control) { control->Gradient = theme::cpu; }));
			pctLabel_ = grid->AddChild(0, 2, makeLabel(L"0%"));
		}));
		
		grid->AddChild(1, 0, init<Grid>([&](Grid* grid)
		{
			grid->SetRowDefinitions("Auto");
			grid->SetColumnDefinitions("*,*");
			grid->HorizontalAlignment = HorizontalAlign::Stretch;

			freqLabel_ = grid->AddChild(0, 0, makeLabel(L""));
			uptimeLabel_ = grid->AddChild(0, 1, makeLabel(L""));
			uptimeLabel_->TextAlignment = TextAlign::Right;
		}));

		coreGrid_ = grid->AddChild(2, 0, init<CoreGridControl>([&](CoreGridControl* grid)
		{
			grid->Cores = &history->cores;
		}));
	});

	Content = init<Grid>([&](Grid* grid)
	{
		grid->SetColumnDefinitions("50*,50*");
		grid->HorizontalAlignment = HorizontalAlign::Stretch;
		grid->VerticalAlignment = VerticalAlign::Stretch;

		grid->AddChild(0, 0, std::move(left));
		grid->AddChild(0, 1, std::move(right));
	});
}

void btop::CpuWidget::SetGraphSymbol(GraphSymbol symbol)
{
	graphUpper_->Symbol = symbol;
	graphLower_->Symbol = symbol;
	coreGrid_->Symbol = symbol;

	graphUpper_->InvalidateVisual();
	graphLower_->InvalidateVisual();
	coreGrid_->InvalidateVisual();
}

void btop::CpuWidget::Update(const Snapshot& snap)
{
	meter_->Value = snap.cpu.totalPercent;
	meter_->InvalidateVisual();

	pctLabel_->Text = std::to_wstring(snap.cpu.totalPercent) + L'%';
	pctLabel_->ForegroundColor = theme::cpu.At(snap.cpu.totalPercent);

	if (snap.cpu.freqMhz > 0)
	{
		wchar_t buf[32] = {};
		swprintf(buf, 32, L"%.2f GHz", snap.cpu.freqMhz / 1000.0);
		freqLabel_->Text = buf;
	}

	uptimeLabel_->Text = L"Uptime: " + secToDhms(snap.power.uptimeSec);

	graphUpper_->InvalidateVisual();
	graphLower_->InvalidateVisual();
	coreGrid_->InvalidateVisual();
}
