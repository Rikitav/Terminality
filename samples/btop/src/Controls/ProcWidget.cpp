#include "ProcWidget.hpp"

#include <algorithm>
#include <cwctype>

using namespace terminality;

namespace
{
	struct ColumnLayout
	{
		int pidX, nameX, thrX, userX, memX, cpuX;
		int pidW = 7, thrW = 5, memW = 9, cpuW = 7;
		int nameW, userW;
	};

	bool contains(const std::wstring& hay, const std::wstring& needle)
	{
		std::wstring lower = hay;
		std::transform(lower.begin(), lower.end(), lower.begin(), std::towlower);
		return lower.find(needle) != std::wstring::npos;
	};

	ColumnLayout layoutFor(int width)
	{
		ColumnLayout l;
		const int fixed = l.pidW + l.thrW + l.memW + l.cpuW + 6; // inter-column spaces
		const int flex = std::max(10, width - fixed);

		l.nameW = std::max(8, flex * 3 / 5);
		l.userW = std::max(6, flex - l.nameW);
		l.pidX = 0;
		l.nameX = l.pidX + l.pidW + 1;
		l.thrX = l.nameX + l.nameW + 1;
		l.userX = l.thrX + l.thrW + 1;
		l.memX = l.userX + l.userW + 1;
		l.cpuX = l.memX + l.memW + 1;
		
		return l;
	}

	std::wstring trim(const std::wstring& text, int width)
	{
		if (static_cast<int>(text.size()) <= width)
			return text;
		
		if (width <= 1)
			return text.substr(0, std::max(0, width));
		
		return text.substr(0, width - 1) + L'\x2026';
	}

	bool matchesFilter(const btop::ProcInfo& proc, const std::wstring& filter)
	{
		if (filter.empty())
			return true;

		std::wstring needle = filter;
		std::transform(needle.begin(), needle.end(), needle.begin(), std::towlower);
		
		return contains(proc.name, needle)
			|| contains(proc.user, needle)
			|| contains(std::to_wstring(proc.pid), needle);
	}
}

void btop::ProcListControl::SetData(std::shared_ptr<const Snapshot> snap)
{
	if (Paused)
		return;

	source_ = std::move(snap);
	Resort();
}

void btop::ProcListControl::Resort()
{
	display_.clear();
	if (source_ != nullptr)
	{
		display_.reserve(source_->procs.size());
		for (const auto& proc : source_->procs)
		{
			if (matchesFilter(proc, Filter))
				display_.push_back(&proc);
		}
	}

	std::stable_sort(display_.begin(), display_.end(), [&](const ProcInfo* a, const ProcInfo* b)
	{
		switch (Sort)
		{
			case ProcSort::Pid:		return a->pid < b->pid;
			case ProcSort::Name:	return a->name < b->name;
			case ProcSort::Threads:	return a->threads < b->threads;
			case ProcSort::User:	return a->user < b->user;
			case ProcSort::Memory:	return a->mem < b->mem;
			case ProcSort::Cpu:		return a->cpu < b->cpu;
		}

		return false;
	});
	
	if (Reversed)
		std::reverse(display_.begin(), display_.end());

	clampSelection();
	InvalidateVisual();
}

const std::wstring& btop::ProcListControl::SortName() const
{
	static const std::wstring names[] = { L"pid", L"program", L"threads", L"user", L"memory", L"cpu" };
	return names[static_cast<int>(Sort)];
}

void btop::ProcListControl::clampSelection()
{
	if (display_.empty())
	{
		selectedPid_ = 0;
		scroll_ = 0;
		return;
	}

	int index = 0;
	if (selectedPid_ != 0)
	{
		for (int i = 0; i < static_cast<int>(display_.size()); ++i)
		{
			if (display_[i]->pid == selectedPid_)
			{
				index = i;
				break;
			}
		}
	}

	const Rect rect = GetArrangedRect();
	const int viewRows = std::max(1, rect.Height - 1);

	if (index < scroll_)
		scroll_ = index;

	if (index >= scroll_ + viewRows)
		scroll_ = index - viewRows + 1;
	
	scroll_ = std::clamp(scroll_, 0, std::max(0, static_cast<int>(display_.size()) - viewRows));
}

uint32_t btop::ProcListControl::SelectedPid() const
{
	if (selectedPid_ != 0)
		return selectedPid_;

	if (!display_.empty() && scroll_ < static_cast<int>(display_.size()))
		return display_[scroll_]->pid;
	
	return 0;
}

int btop::ProcListControl::DisplayCount() const
{
	return static_cast<int>(display_.size());
}

Size btop::ProcListControl::MeasureOverride(const Size& availableSize)
{
	int32_t width = availableSize.Width >= 0
		? std::max(availableSize.Width, 1)
		: 60;
	
	int32_t height = availableSize.Height >= 0
		? std::max(availableSize.Height, 1)
		: 10;
	
	return Size(width, height);
}

bool btop::ProcListControl::OnKeyDown(InputEvent input)
{
	if (display_.empty())
		return false;

	int index = -1;
	for (int i = 0; i < static_cast<int>(display_.size()); ++i)
	{
		if (display_[i]->pid == selectedPid_)
		{
			index = i;
			break;
		}
	}

	if (index < 0)
		index = scroll_;

	const Rect rect = GetArrangedRect();
	const int viewRows = std::max(1, rect.Height - 1);

	switch (input.Key)
	{
		case InputKey::UP:
		{
			index = std::max(0, index - 1);
			break;
		}
		
		case InputKey::DOWN:
		{
			index = std::min(static_cast<int>(display_.size()) - 1, index + 1);
			break;
		}
		
		case InputKey::PRIOR:
		{
			index = std::max(0, index - viewRows);
			break;
		}
		
		case InputKey::NEXT:
		{
			index = std::min(static_cast<int>(display_.size()) - 1, index + viewRows);
			break;
		}
		
		case InputKey::HOME:
		{
			index = 0;
			break;
		}
		
		case InputKey::END:
		{
			index = static_cast<int>(display_.size()) - 1;
			break;
		}

		case InputKey::LEFT:
		{
			Sort = static_cast<ProcSort>((static_cast<int>(Sort) + ProcSortCount - 1) % ProcSortCount);
			Resort();
			return true;
		}
		
		case InputKey::RIGHT:
		{
			Sort = static_cast<ProcSort>((static_cast<int>(Sort) + 1) % ProcSortCount);
			Resort();
			return true;
		}
		
		default:
			return false;
	}

	selectedPid_ = display_[index]->pid;
	clampSelection();
	InvalidateVisual();
	return true;
}

void btop::ProcListControl::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	if (rect.Width < 30 || rect.Height < 2)
		return;

	const ColumnLayout l = layoutFor(rect.Width);
	const Color bg = Color::BLACK;
	const int viewRows = rect.Height - 1;

	// Header.
	context.RenderText(Point(l.pidX, 0),  L"PID", theme::title, bg);
	context.RenderText(Point(l.nameX, 0), L"PROGRAM", theme::title, bg);
	context.RenderText(Point(l.thrX, 0),  L"THR", theme::title, bg);
	context.RenderText(Point(l.userX, 0), L"USER", theme::title, bg);
	context.RenderText(Point(l.memX, 0),  MemAsPercent ? L"MEM%" : L"MEMB", theme::title, bg);
	context.RenderText(Point(l.cpuX, 0),  L"CPU%", theme::title, bg);

	if (display_.empty())
	{
		context.RenderText(Point(0, 1), Filter.empty() ? L"No processes" : L"No matching processes", theme::inactiveFg, bg);
		return;
	}

	clampSelection();
	for (int row = 0; row < viewRows; ++row)
	{
		const int index = scroll_ + row;
		const int y = 1 + row;
		if (index >= static_cast<int>(display_.size()))
			break;

		const ProcInfo& proc = *display_[index];
		const bool selected = proc.pid == selectedPid_ || (selectedPid_ == 0 && row == 0);
		const Color rowBg = selected ? theme::selectedBg : bg;
		const Color rowFg = selected ? theme::selectedFg : theme::mainFg;

		if (selected)
		{
			for (int x = 0; x < rect.Width; ++x)
				context.SetCell(x, y, L' ', theme::selectedFg, theme::selectedBg);
		}

		wchar_t buf[32] = {};
		swprintf(buf, 32, L"%7u", proc.pid);
		context.RenderText(Point(l.pidX, y), buf, rowFg, rowBg);
		context.RenderText(Point(l.nameX, y), trim(proc.name, l.nameW), rowFg, rowBg);

		swprintf(buf, 32, L"%5u", proc.threads);
		context.RenderText(Point(l.thrX, y), buf, theme::procMisc, rowBg);
		context.RenderText(Point(l.userX, y), trim(proc.user, l.userW), rowFg, rowBg);

		std::wstring memText = (MemAsPercent && TotalMem > 0)
			? std::to_wstring(static_cast<int64_t>(proc.mem * 100 / TotalMem)) + L'%'
			: humanize(proc.mem, true);

		context.RenderText(Point(l.memX, y), trim(memText, l.memW),
			theme::process.At(static_cast<int64_t>(proc.mem * 100 / std::max<uint64_t>(TotalMem, 1))), rowBg);

		swprintf(buf, 32, L"%6.1f", proc.cpu);
		context.RenderText(Point(l.cpuX, y), std::wstring(buf),
			theme::process.At(static_cast<int64_t>(std::clamp(proc.cpu, 0.0, 100.0))), rowBg);
	}
}

btop::ProcWidget::ProcWidget()
{
	HeaderText = L" PROC ";
	BorderColor = theme::procBox;
	ForegroundColor = theme::title;
	BackgroundColor = theme::mainBg;
	Padding = Thickness(0, 0, 1, 0);

	auto list = init<ProcListControl>([](ProcListControl*) {});
	list_ = list.get();
	Content = std::move(list);
}

void btop::ProcWidget::Update(const std::shared_ptr<const Snapshot>& snap)
{
	list_->TotalMem = snap->mem.total;
	list_->SetData(snap);
	UpdateTitle();
}

void btop::ProcWidget::TogglePause()
{
	list_->Paused = !list_->Paused;
	UpdateTitle();
}

void btop::ProcWidget::SetFilter(const std::wstring& filter)
{
	list_->Filter = filter;
	list_->Resort();
	UpdateTitle();
}

void btop::ProcWidget::UpdateTitle()
{
	if (list_->Paused)
	{
		HeaderText = L" PROC - PAUSED ";
		return;
	}
	
	if (!list_->Filter.empty())
	{
		HeaderText = L" PROC - filter: " + list_->Filter + L" ";
		return;
	}

	HeaderText = L" PROC - " + list_->SortName() + (list_->Reversed ? L" (reversed) " : L" ");
}
