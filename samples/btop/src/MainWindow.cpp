#include "MainWindow.hpp"

#include <algorithm>
#include <cstdio>

#include "Controls/SearchBox.hpp"
#include "Controls/Theme.hpp"
#include "Services/NetService.hpp"

using namespace terminality;

namespace
{
	// Header divider: renders only the bottom edge, like btop's div_line under the header.
	wchar_t headerDividerStyle(const RectanglePos pos)
	{
		switch (pos)
		{
			case RectanglePos::BottomHorizontalLine: return L'\x2500';
			default: return L' ';
		}
	}

	std::wstring clockString()
	{
		SYSTEMTIME time{};
		GetLocalTime(&time);
		wchar_t buf[16] = {};
		swprintf(buf, 16, L"%02u:%02u:%02u", time.wHour, time.wMinute, time.wSecond);
		return buf;
	}

	std::unique_ptr<Label> makeLabel(const std::wstring& text, btop::Color color = btop::theme::mainFg)
	{
		return init<Label>([&](Label* label)
		{
			label->Text = text;
			label->ForegroundColor = color;
			label->BackgroundColor = btop::theme::mainBg;
		});
	}

	VisualTreeNode* findFirstFocusable(VisualTreeNode* node)
	{
		if (node == nullptr)
			return nullptr;

		if (auto* control = dynamic_cast<ControlBase*>(node))
		{
			if (control->IsFocusable() && control->IsTabStop() && control->VisualChildrenCount() == 0)
				return node;
		}
		
		for (std::size_t i = 0; i < node->VisualChildrenCount(); ++i)
		{
			if (VisualTreeNode* found = findFirstFocusable(node->GetVisualChild(i)))
				return found;
		}

		return nullptr;
	}

	void pushModal(std::unique_ptr<ControlBase> content, const std::wstring& header)
	{
		auto container = init<Grid>([&](Grid* grid)
		{
			grid->HorizontalAlignment = HorizontalAlign::Center;
			grid->VerticalAlignment = VerticalAlign::Center;
			grid->BackgroundColor = Color::TRANSPARENT;
			grid->FocusedBackgroundColor = Color::TRANSPARENT;
			grid->SetRowDefinitions("*");

			grid->OnHotkey(InputModifier::None, InputKey::ESCAPE, [](ControlBase* self)
			{
				self->Close();
			});

			grid->AddChild(0, 0, init<Border>([&](Border* border)
			{
				border->HeaderText = header;
				border->BorderColor = btop::theme::hiFg;
				border->ForegroundColor = btop::theme::title;
				border->BackgroundColor = btop::theme::mainBg;
				border->Padding = Thickness(2, 1, 2, 1);
				border->HorizontalAlignment = HorizontalAlign::Center;
				border->VerticalAlignment = VerticalAlign::Center;
				border->Content = std::move(content);

				border->OnHotkey(InputModifier::None, InputKey::ESCAPE, [](ControlBase* self)
				{
					self->Close();
				});
			}));
		});

		UILayer& layer = VisualTree::Current().PushLayer(std::move(container));
		if (VisualTreeNode* first = findFirstFocusable(layer.RootNode.get()))
			layer.Focus.SetFocused(first);

		HostApplication::Current().NestUILoop(layer);
		VisualTree::Current().PopLayer();
	}
}

btop::MainWindow::MainWindow()
{
	DispatchTimer::Current().SetUIThread();

	BackgroundColor = theme::mainBg;
	FocusedBackgroundColor = theme::mainBg;
	HorizontalAlignment = HorizontalAlign::Stretch;
	VerticalAlignment = VerticalAlign::Stretch;

	SetRowDefinitions("Auto,*");
	AddChild(0, 0, buildHeader());

	auto boxes = std::make_unique<BoxesGrid>(&history_);
	boxes_ = boxes.get();
	AddChild(1, 0, std::move(boxes));

	registerHotkeys();

	DispatchTimer::Current().TickEvent += [this](float dt) { onTick(dt); };

	collector_.Start(updateMs_, [this](std::shared_ptr<const Snapshot> snap)
	{
		DispatchTimer::Current().InvokeAsync([this, snap]() mutable
		{
			if (alive_)
				onSnapshot(std::move(snap));
		});
	});
}

btop::MainWindow::~MainWindow()
{
	alive_ = false;
	collector_.Stop();
}

std::unique_ptr<ControlBase> btop::MainWindow::buildHeader()
{
	return init<Border>([&](Border* border)
	{
		border->Style = headerDividerStyle;
		border->BorderColor = theme::divLine;
		border->BackgroundColor = theme::mainBg;

		border->Content = init<Grid>([&](Grid* grid)
		{
			grid->SetRowDefinitions("1");
			grid->SetColumnDefinitions("Auto,*,Auto");
			grid->HorizontalAlignment = HorizontalAlign::Stretch;
			grid->VerticalAlignment = VerticalAlign::Stretch;

			grid->AddChild(0, 0, init<StackPanel>([](StackPanel* panel)
			{
				panel->ContentOrientation = Orientation::Horizontal;
				panel->ItemSpacing = 2;

				panel->AddChild(makeLabel(L" btop++", theme::title));
				panel->AddChild(makeLabel(L"[m]enu"));
				panel->AddChild(makeLabel(L"[h]elp"));
			}));

			clockLabel_ = grid->AddChild(0, 1, makeLabel(clockString(), theme::title));
			clockLabel_->TextAlignment = TextAlign::Center;
			
			grid->AddChild(0, 2, init<StackPanel>([&](StackPanel* panel)
			{
				panel->ContentOrientation = Orientation::Horizontal;
				panel->ItemSpacing = 2;

				updateLabel_ = panel->AddChild(makeLabel(L""));
				batteryLabel_ = panel->AddChild(makeLabel(L""));
			}));
		});
	});
}

void btop::MainWindow::registerHotkeys()
{
	registerSystemHotkeys();
	registerBoxHotkeys();
	registerTimerHotkeys();
	registerNetHotkeys();
	registerProcHotkeys();
}

void btop::MainWindow::registerSystemHotkeys()
{
	OnHotkey(InputModifier::None, InputKey::Q, [](ControlBase*) { HostApplication::Current().RequestStop(); });
	OnHotkey(InputModifier::None, InputKey::M, [this](ControlBase*) { showMainMenu(); });
	OnHotkey(InputModifier::None, InputKey::ESCAPE, [this](ControlBase*) { showMainMenu(); });
	OnHotkey(InputModifier::None, InputKey::H, [this](ControlBase*) { showHelp(); });
	OnHotkey(InputModifier::None, InputKey::F1, [this](ControlBase*) { showHelp(); });
	OnHotkey(InputModifier::None, InputKey::OEM_2, [this](ControlBase*) { showHelp(); });
}

void btop::MainWindow::registerBoxHotkeys()
{
	auto toggleBox = [this](int index)
	{
		boxes_->SetBoxVisible(index, !boxes_->IsBoxVisible(index));
		boxes_->Relayout();
	};

	OnHotkey(InputModifier::None, InputKey::NUM1, [toggleBox](ControlBase*) { toggleBox(0); });
	OnHotkey(InputModifier::None, InputKey::NUM2, [toggleBox](ControlBase*) { toggleBox(1); });
	OnHotkey(InputModifier::None, InputKey::NUM3, [toggleBox](ControlBase*) { toggleBox(2); });
	OnHotkey(InputModifier::None, InputKey::NUM4, [toggleBox](ControlBase*) { toggleBox(3); });
}

void btop::MainWindow::registerTimerHotkeys()
{
	OnHotkey(InputModifier::None, InputKey::OEM_PLUS, [this](ControlBase*)
	{
		updateMs_ = std::min(updateMs_ + 100, 10000);
		collector_.SetInterval(updateMs_);
		updateHeader();
	});

	OnHotkey(InputModifier::None, InputKey::OEM_MINUS, [this](ControlBase*)
	{
		updateMs_ = std::max(updateMs_ - 100, 100);
		collector_.SetInterval(updateMs_);
		updateHeader();
	});
}

void btop::MainWindow::registerNetHotkeys()
{
	OnHotkey(InputModifier::None, InputKey::B, [this](ControlBase*)
	{
		NetService::NextIface(-1);
		collector_.RequestImmediate();
	});

	OnHotkey(InputModifier::None, InputKey::N, [this](ControlBase*)
	{
		NetService::NextIface(1);
		collector_.RequestImmediate();
	});

	OnHotkey(InputModifier::None, InputKey::Z, [this](ControlBase*)
	{
		NetService::ZeroTotals();
		collector_.RequestImmediate();
	});

	OnHotkey(InputModifier::None, InputKey::A, [this](ControlBase*)
	{
		boxes_->Net()->ToggleAutoScale();
	});
}

void btop::MainWindow::registerProcHotkeys()
{
	OnHotkey(InputModifier::None, InputKey::F, [this](ControlBase*)
	{
		showFilter();
	});

	OnHotkey(InputModifier::None, InputKey::K, [this](ControlBase*)
	{
		killSelected();
	});

	OnHotkey(InputModifier::None, InputKey::U, [this](ControlBase*)
	{
		boxes_->Proc()->TogglePause();
	});
	
	OnHotkey(InputModifier::None, InputKey::R, [this](ControlBase*)
	{
		auto* list = boxes_->Proc()->List();
		list->Reversed = !list->Reversed;
		list->Resort();
	});

	OnHotkey(InputModifier::Shift, InputKey::NUM5, [this](ControlBase*)
	{
		auto* list = boxes_->Proc()->List();
		list->MemAsPercent = !list->MemAsPercent;
		list->InvalidateVisual();
	});
}

void btop::MainWindow::onTick(float dt)
{
	SYSTEMTIME time{};
	GetLocalTime(&time);
	if (static_cast<int>(time.wSecond) != lastClockSecond_)
	{
		lastClockSecond_ = static_cast<int>(time.wSecond);
		updateHeader();
	}
}

void btop::MainWindow::onSnapshot(std::shared_ptr<const Snapshot> snap)
{
	snap_ = std::move(snap);
	pushHistories(*snap_);

	boxes_->Cpu()->Update(*snap_);
	boxes_->Mem()->Update(*snap_);
	boxes_->Net()->Update(*snap_);
	boxes_->Proc()->Update(snap_);

	updateHeader();
}

void btop::MainWindow::pushHistories(const Snapshot& snap)
{
	history_.ensureCores(snap.cpu.coreCount);
	History::push(history_.cpuTotal, snap.cpu.totalPercent);
	for (size_t i = 0; i < snap.cpu.coreCount && i < history_.cores.size(); ++i)
		History::push(history_.cores[i], snap.cpu.corePercent[i], 40);

	History::push(history_.netDown, static_cast<int64_t>(snap.net.downSpeed));
	History::push(history_.netUp, static_cast<int64_t>(snap.net.upSpeed));

	auto pct = [](uint64_t part, uint64_t total) -> int64_t
	{
		return total > 0 ? static_cast<int64_t>(part * 100 / total) : 0;
	};

	History::push(history_.memUsed, pct(snap.mem.used, snap.mem.total));
	History::push(history_.memAvailable, pct(snap.mem.available, snap.mem.total));
	History::push(history_.memCached, pct(snap.mem.cached, snap.mem.total));
	History::push(history_.memFree, pct(snap.mem.free, snap.mem.total));
	History::push(history_.swapUsed, pct(snap.mem.swapUsed, snap.mem.swapTotal));
	History::push(history_.swapFree, pct(snap.mem.swapFree, snap.mem.swapTotal));
}

void btop::MainWindow::updateHeader()
{
	if (clockLabel_ != nullptr)
		clockLabel_->Text = clockString();

	if (updateLabel_ != nullptr)
		updateLabel_->Text = L"update: " + std::to_wstring(updateMs_) + L"ms [-/+]";

	if (batteryLabel_ != nullptr && snap_ != nullptr)
	{
		const PowerData& power = snap_->power;
		if (power.hasBattery)
		{
			batteryLabel_->Text = (power.batteryOnAc ? L"BAT \x25B2 " : L"BAT \x25BC ") + std::to_wstring(power.batteryPercent) + L'%';
			batteryLabel_->ForegroundColor = theme::title;
		}
		else
		{
			batteryLabel_->Text = L"";
		}
	}
}

void btop::MainWindow::showMainMenu()
{
	auto content = init<StackPanel>([this](StackPanel* panel)
	{
		panel->ContentOrientation = Orientation::Vertical;
		panel->HorizontalContentAlignment = HorizontalAlign::Stretch;

		panel->AddChild(init<Button>([this](Button* button)
		{
			button->Text = L"&Options";
			button->Clicked += [this]() { showOptions(); };
		}));

		panel->AddChild(init<Button>([this, panel](Button* button)
		{
			button->Text = L"&Help";
			button->Clicked += [this, panel]()
			{
				panel->Close();
				showHelp();
			};
		}));
		
		panel->AddChild(init<Button>([](Button* button)
		{
			button->Text = L"&Quit";
			button->Clicked += []() { HostApplication::Current().RequestStop(); };
		}));
	});

	pushModal(std::move(content), L" Menu ");
}

void btop::MainWindow::showOptions()
{
	auto content = init<StackPanel>([this](StackPanel* panel)
	{
		panel->ContentOrientation = Orientation::Vertical;
		panel->ItemSpacing = 1;
		panel->HorizontalContentAlignment = HorizontalAlign::Left;

		auto updateRow = init<StackPanel>([this](StackPanel* row)
		{
			row->ContentOrientation = Orientation::Horizontal;
			row->ItemSpacing = 1;
			row->HorizontalContentAlignment = HorizontalAlign::Left;
			row->AddChild(makeLabel(L"Update timer:"));

			Label* updateRaw = row->AddChild(makeLabel(std::to_wstring(updateMs_) + L" ms"));

			row->AddChild(init<Button>([this, updateRaw](Button* button)
			{
				button->Text = L"-";
				button->Clicked += [this, updateRaw]()
				{
					updateMs_ = std::max(updateMs_ - 100, 100);
					updateRaw->Text = std::to_wstring(updateMs_) + L" ms";
					collector_.SetInterval(updateMs_);
					updateHeader();
				};
			}));

			row->AddChild(init<Button>([this, updateRaw](Button* button)
			{
				button->Text = L"+";
				button->Clicked += [this, updateRaw]()
				{
					updateMs_ = std::min(updateMs_ + 100, 10000);
					updateRaw->Text = std::to_wstring(updateMs_) + L" ms";
					collector_.SetInterval(updateMs_);
					updateHeader();
				};
			}));
		});

		auto symbolRow = init<StackPanel>([this](StackPanel* row)
		{
			row->ContentOrientation = Orientation::Horizontal;
			row->ItemSpacing = 1;
			row->HorizontalContentAlignment = HorizontalAlign::Left;

			row->AddChild(makeLabel(L"Graph symbol:"));
			row->AddChild(init<Button>([this](Button* button)
			{
				button->Text = graphSymbol_ == GraphSymbol::Braille ? L"braille" : L"block";
				button->Clicked += [this, button]()
				{
					graphSymbol_ = graphSymbol_ == GraphSymbol::Braille ? GraphSymbol::Block : GraphSymbol::Braille;
					applyGraphSymbol(graphSymbol_);
					button->Text = graphSymbol_ == GraphSymbol::Braille ? L"braille" : L"block";
				};
			}));
		});

		panel->AddChild(std::move(updateRow));
		panel->AddChild(std::move(symbolRow));
	});

	pushModal(std::move(content), L" Options ");
}

void btop::MainWindow::showHelp()
{
	static const wchar_t* helpText =
		L"q                    Quit\n"
		L"m, esc               Main menu\n"
		L"h, ?, F1             Help\n"
		L"1, 2, 3, 4           Toggle cpu, mem, net, proc boxes\n"
		L"+, -                 Update timer +100ms / -100ms\n"
		L"\n"
		L"CPU box:\n"
		L"  Show history graph of total and per-core usage.\n"
		L"\n"
		L"NET box:\n"
		L"b, n                 Previous / next network interface\n"
		L"z                    Zero network totals\n"
		L"a                    Toggle auto-scaling of network graphs\n"
		L"\n"
		L"PROC box:\n"
		L"up/down, pgup/pgdn   Select process\n"
		L"home, end            Jump to first / last process\n"
		L"left, right          Change sort column\n"
		L"r                    Reverse sort order\n"
		L"f                    Filter processes\n"
		L"u                    Pause / resume updates\n"
		L"k                    Terminate selected process\n"
		L"shift+5              Toggle MEMB column bytes / percent";

	pushModal(makeLabel(helpText), L" Help ");
}

void btop::MainWindow::showFilter()
{
	auto content = init<StackPanel>([this](StackPanel* panel)
	{
		panel->ContentOrientation = Orientation::Vertical;
		panel->HorizontalAlignment = HorizontalAlign::Center;
		panel->VerticalAlignment = VerticalAlign::Center;
		panel->MinSize = Size(10, 1);

		panel->AddChild(init<SearchBox>([this](SearchBox* box)
		{
			box->Text = boxes_->Proc()->List()->Filter;
			box->PlaceholderText = L"Filter by name, user or pid (empty = all)";
			box->HorizontalAlignment = HorizontalAlign::Stretch;
			box->Submitted += [this, box]()
			{
				boxes_->Proc()->SetFilter(box->Text.Get());
				box->Close();
			};
		}));
	});

	pushModal(std::move(content), L" Filter ");
}

void btop::MainWindow::applyGraphSymbol(GraphSymbol symbol)
{
	boxes_->Cpu()->SetGraphSymbol(symbol);
	boxes_->Net()->SetGraphSymbol(symbol);
	boxes_->Mem()->SetGraphSymbol(symbol);
}

void btop::MainWindow::killSelected()
{
	const uint32_t pid = boxes_->Proc()->SelectedPid();
	if (pid == 0)
		return;

	const std::wstring message = L"Terminate process " + std::to_wstring(pid) + L"?";
	if (MessageBox::Show(L"Kill process", message, MessageBoxButton::YesNo) != MessageBoxResult::Yes)
		return;

	const HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
	if (process == nullptr)
	{
		MessageBox::Show(L"Kill process", L"Failed to open the process.", MessageBoxButton::Ok);
		return;
	}

	if (!TerminateProcess(process, 1))
		MessageBox::Show(L"Kill process", L"Failed to terminate the process.", MessageBoxButton::Ok);
	
	CloseHandle(process);
}
