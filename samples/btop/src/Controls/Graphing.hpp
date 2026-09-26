#pragma once

#include <deque>

#include "Common.hpp"
#include "Theme.hpp"

// Braille/block history graphs, ported from btop's Draw::Graph (btop_draw.cpp):
// * two samples share one glyph column,
// * each sample quantized to 5 vertical levels per text row,
// * rows colored from a theme gradient.
namespace btop
{
	enum class GraphSymbol
	{
		Braille,
		Block,
	};

	// Draws the tail of `data` into the rectangle. Values are 0..100 unless
	// maxValue > 0, in which case they are scaled to 0..100 by value/maxValue.
	void DrawGraph(
		terminality::RenderContext& context,
		const terminality::Rect& rect,
		const std::deque<int64_t>& data,
		const Gradient& gradient,
		GraphSymbol symbol,
		bool invert,
		int64_t maxValue,
		bool noZero = false
	);

	// Solid block percent meter (btop's Draw::Meter).
	void DrawMeter(
		terminality::RenderContext& context,
		const terminality::Rect& rect,
		const Gradient& gradient,
		int64_t value,
		bool invert = false
	);

	// A self-rendering history graph bound to a shared deque.
	class GraphControl : public terminality::ControlBase
	{
	public:
		const std::deque<int64_t>* Data = nullptr;
		Gradient Gradient = theme::cpu;
		GraphSymbol Symbol = GraphSymbol::Braille;
		bool Invert = false;
		bool NoZero = false;
		int64_t MaxValue = 0; // 0 -> data already 0..100

		GraphControl()
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

	// Horizontal percent meter (btop's Draw::Meter):
	// * solid blocks colored from the gradient,
	// * empty part rendered in meterBg.
	class MeterControl : public terminality::ControlBase
	{
	public:
		MeterControl() { SetFocusable(false); }

		Gradient Gradient = theme::cpu;
		int64_t Value = 0; // 0..100
		bool Invert = false;

	protected:
		terminality::Size MeasureOverride(const terminality::Size& availableSize) override;
		void ArrangeOverride(const terminality::Rect& contentRect) override { }
		void RenderOverride(terminality::RenderContext& context) override;

		std::size_t VisualChildrenCount() const override { return 0; }
		terminality::VisualTreeNode* GetVisualChild(std::size_t) const override { return nullptr; }
	};
}
