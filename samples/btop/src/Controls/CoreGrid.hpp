#pragma once

#include <vector>
#include <deque>

#include "Common.hpp"
#include "Graphing.hpp"

namespace btop
{
	class CoreGridControl : public terminality::ControlBase
	{
	public:
		const std::vector<std::deque<int64_t>>* Cores = nullptr;
		GraphSymbol Symbol = GraphSymbol::Braille;

		CoreGridControl()
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
