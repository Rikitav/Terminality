#pragma once

#include <deque>
#include <string>
#include <vector>

#include "Common.hpp"
#include "Graphing.hpp"
#include "Theme.hpp"

// One "label [sparkline] value" row of the MEM box (btop's mem box rows).
namespace btop
{
	struct StatRow
	{
		std::wstring label;
		const std::deque<int64_t>* history = nullptr; // percent 0..100
		int64_t percent = 0;
		uint64_t bytes = 0;
		Gradient gradient = theme::used;
		bool isHeader = false; // "Swap:" style sub-header
	};

	class StatRowsControl : public terminality::ControlBase
	{
	public:
		std::vector<StatRow> Rows;
		GraphSymbol Symbol = GraphSymbol::Braille;

		StatRowsControl()
		{
			SetFocusable(false);
		}

	protected:
		terminality::Size MeasureOverride(const terminality::Size& availableSize) override;
		void ArrangeOverride(const terminality::Rect& contentRect) override { }
		void RenderOverride(terminality::RenderContext& context) override;

		std::size_t VisualChildrenCount() const override { return 0; }
		terminality::VisualTreeNode* GetVisualChild(std::size_t) const override { return nullptr; }
	};
}
