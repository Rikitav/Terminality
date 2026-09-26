#include "NetWidget.hpp"

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

	constexpr int64_t ManualScaleBytes = (100ll << 20) / 8; // btop's 100 Mib default
}

btop::NetWidget::NetWidget(History* history)
	: history_(history)
{
	HeaderText = L" NET ";
	BorderColor = theme::netBox;
	ForegroundColor = theme::title;
	BackgroundColor = theme::mainBg;
	Padding = Thickness(1, 0, 1, 0);

	Content = init<Grid>([&](Grid* grid)
	{
		grid->SetColumnDefinitions("55*,45*");
		grid->HorizontalAlignment = HorizontalAlign::Stretch;
		grid->VerticalAlignment = VerticalAlign::Stretch;
		
		grid->AddChild(0, 0, init<Grid>([&](Grid* grid)
		{
			grid->SetRowDefinitions("50*,50*");
			grid->HorizontalAlignment = HorizontalAlign::Stretch;
			grid->VerticalAlignment = VerticalAlign::Stretch;

			downGraph_ = grid->AddChild(0, 0, init<GraphControl>([history](GraphControl* graph)
			{
				graph->Data = &history->netDown;
				graph->Gradient = theme::download;
			}));
			
			upGraph_ =  grid->AddChild(1, 0, init<GraphControl>([history](GraphControl* graph)
			{
				graph->Data = &history->netUp;
				graph->Gradient = theme::upload;
			}));
		}));
		
		grid->AddChild(0, 1, init<StackPanel>([&](StackPanel* panel)
		{
			panel->ContentOrientation = Orientation::Vertical;
			panel->ItemSpacing = 1;
			
			ifaceLabel_ = panel->AddChild(makeLabel(L""));
			downInfo_ = panel->AddChild(makeLabel(L""));
			upInfo_ = panel->AddChild(makeLabel(L""));
		}));
	});
}

void btop::NetWidget::Update(const Snapshot& snap)
{
	const NetData& net = snap.net;
	std::wstring iface = L"\x25C6 " + std::wstring(net.iface.begin(), net.iface.end());

	if (!net.ipv4.empty())
		iface += L"  " + net.ipv4;

	ifaceLabel_->Text = iface;
	downInfo_->Text = L"\x25BC " + humanize(net.downSpeed, false, true)
		+ L"  Total: " + humanize(net.downTotal)
		+ L"\nTop: " + humanize(net.downTop, true);
	
	upInfo_->Text = L"\x25B2 " + humanize(net.upSpeed, false, true)
		+ L"  Total: " + humanize(net.upTotal)
		+ L"\nTop: " + humanize(net.upTop, true);

	AdaptScale(snap);
	
	downGraph_->MaxValue = autoScale_ ? maxDown_ : ManualScaleBytes;
	upGraph_->MaxValue = autoScale_ ? maxUp_ : ManualScaleBytes;

	downGraph_->InvalidateVisual();
	upGraph_->InvalidateVisual();
}

void btop::NetWidget::SetGraphSymbol(GraphSymbol symbol)
{
	downGraph_->Symbol = symbol;
	upGraph_->Symbol = symbol;

	downGraph_->InvalidateVisual();
	upGraph_->InvalidateVisual();
}

void btop::NetWidget::ToggleAutoScale()
{
	autoScale_ = !autoScale_;
	if (autoScale_)
	{
		maxDown_ = 10 << 10;
		maxUp_ = 10 << 10;
	}
}

void btop::NetWidget::AdaptScale(const Snapshot& snap)
{
	if (!autoScale_)
		return;

	// btop's net_auto: grow to avg(last 5) * headroom, shrink when the average drops below a tenth of the current scale.
	auto adapt = [](const std::deque<int64_t>& hist, int64_t& maxValue, double multiplier)
	{
		int64_t sum = 0;
		
		const size_t count = std::min<size_t>(5, hist.size());
		for (size_t i = hist.size() - count; i < hist.size(); ++i)
			sum += hist[i];
		
		const int64_t avg = count > 0
			? sum / static_cast<int64_t>(count)
			: 0;
	
		const int64_t target = std::max<int64_t>(static_cast<int64_t>(avg * multiplier), 10 << 10);
		if (target > maxValue || avg < maxValue / 10)
			maxValue = target;
	};

	adapt(history_->netDown, maxDown_, 3.0);
	adapt(history_->netUp, maxUp_, 1.3);
}
