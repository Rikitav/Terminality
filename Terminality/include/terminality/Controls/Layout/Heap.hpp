#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include <terminality/Core/Geometry.hpp>
#include <terminality/Core/Focus.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Framework/Property.hpp>

namespace terminality
{
	/// Absolute-positioning container. Each child is "pinned" to an explicit
	/// point; the heap measures to the bounding box of its children.
	class Heap : public ControlBase
	{
	public:
		struct HeapChild
		{
			std::unique_ptr<ControlBase> Control;
			Point Position;
		};

	protected:
		std::vector<HeapChild> children_;
		std::size_t focusedIndex_ = 0;

	public:
		Heap() = default;

		// Adds a child at the given point and returns a raw pointer to it.
		ControlBase* AddChild(Point position, std::unique_ptr<ControlBase> child);
		ControlBase* AddChild(int32_t x, int32_t y, std::unique_ptr<ControlBase> child);

		std::unique_ptr<ControlBase> RemoveChild(ControlBase* child);
		void Clear();

		std::size_t Count() const;

		bool MoveFocusNext(Direction direction, InputModifier modifiers) override;
		void OnGotFocus() override;
		void OnLostFocus() override;

		std::size_t VisualChildrenCount() const override;
		VisualTreeNode* GetVisualChild(std::size_t index) const override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}
