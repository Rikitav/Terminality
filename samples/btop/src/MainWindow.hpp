#pragma once

#include <atomic>
#include <memory>

#include "Common.hpp"
#include "Controls/BoxesGrid.hpp"
#include "History.hpp"
#include "Services/Collector.hpp"
#include "Services/SystemTypes.hpp"

namespace btop
{
	// Root control: header line, the four box widgets and global keybindings.
	// Owns the collector thread; fresh snapshots arrive on the UI thread
	// through DispatchTimer::InvokeAsync.
	class MainWindow : public terminality::Grid
	{
		std::shared_ptr<const Snapshot> snap_;
		std::atomic<bool> alive_{ true };
		int updateMs_ = 2000;
		int lastClockSecond_ = -1;

		Collector collector_;
		History history_;

		GraphSymbol graphSymbol_ = GraphSymbol::Braille;
		BoxesGrid* boxes_ = nullptr;

		terminality::Label* clockLabel_ = nullptr;
		terminality::Label* updateLabel_ = nullptr;
		terminality::Label* batteryLabel_ = nullptr;

		// UI construction
		std::unique_ptr<terminality::ControlBase> buildHeader();
		void registerHotkeys();
		void registerSystemHotkeys();
		void registerBoxHotkeys();
		void registerTimerHotkeys();
		void registerNetHotkeys();
		void registerProcHotkeys();

		// Runtime
		void onTick(float dt);
		void onSnapshot(std::shared_ptr<const Snapshot> snap);
		void pushHistories(const Snapshot& snap);
		void updateHeader();

		// Modals
		void showMainMenu();
		void showOptions();
		void showHelp();
		void showFilter();
		void killSelected();
		void applyGraphSymbol(GraphSymbol symbol);

	public:
		MainWindow();
		~MainWindow() override;
	};
}
