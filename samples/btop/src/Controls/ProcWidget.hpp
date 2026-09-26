#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Common.hpp"
#include "Services/SystemTypes.hpp"
#include "Theme.hpp"

// The PROC box: sortable, filterable process table with keyboard navigation. Column set mirrors btop's default view: Pid, Program, Threads, User, MemB, Cpu%.
namespace btop
{
	enum class ProcSort
	{
		Pid,
		Name,
		Threads,
		User,
		Memory,
		Cpu,
	};

	constexpr int ProcSortCount = 6;

	class ProcListControl : public terminality::ControlBase
	{
		std::shared_ptr<const Snapshot> source_;                    // last published snapshot (kept alive)
		std::vector<const ProcInfo*> display_;  // filtered + sorted, points into source_
		uint32_t selectedPid_ = 0;
		int scroll_ = 0;

		void rebuild();
		void clampSelection();

	public:
		ProcSort Sort = ProcSort::Cpu;
		bool Reversed = false;
		bool MemAsPercent = false;
		bool Paused = false;
		std::wstring Filter;
		uint64_t TotalMem = 0; // for percent display

		ProcListControl()
		{
			FocusedBackgroundColor = Color::BLACK;
		}

		void Resort();
		
		uint32_t SelectedPid() const;
		int DisplayCount() const;

		const std::wstring& SortName() const;
		void SetData(std::shared_ptr<const Snapshot> snap);

	protected:
		terminality::Size MeasureOverride(const terminality::Size& availableSize) override;
		void ArrangeOverride(const terminality::Rect& contentRect) override { }
		void RenderOverride(terminality::RenderContext& context) override;
		
		bool OnKeyDown(terminality::InputEvent input) override;

		std::size_t VisualChildrenCount() const override { return 0; }
		terminality::VisualTreeNode* GetVisualChild(std::size_t) const override { return nullptr; }
	};

	// Framed PROC box wrapping the process list; owns the box title state.
	class ProcWidget : public terminality::Border
	{
		ProcListControl* list_ = nullptr;

		void UpdateTitle();

	public:
		ProcWidget();

		ProcListControl* List() const { return list_; }

		void Update(const std::shared_ptr<const Snapshot>& snap);
		void TogglePause();

		void SetFilter(const std::wstring& filter);
		
		uint32_t SelectedPid() const { return list_->SelectedPid(); }
		const std::wstring& SortName() const { return list_->SortName(); }
	};
}
