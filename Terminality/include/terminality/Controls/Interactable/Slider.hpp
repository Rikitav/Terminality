#pragma once

#include <terminality/Core/Geometry.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Core/Layout.hpp>
#include <terminality/Engine/RenderContext.hpp>
#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Framework/Event.hpp>
#include <terminality/Framework/Property.hpp>

namespace terminality
{
	/// A draggable value range control. Use Left/Right (or Up/Down when vertical)
	/// to move the handle by Step. Emits ValueChanged with the new value.
	class Slider : public ControlBase
	{
	public:
		Property<Slider, float> Minimum { this, "Minimum", 0.0f, InvalidationKind::Visual };
		Property<Slider, float> Maximum { this, "Maximum", 100.0f, InvalidationKind::Visual };
		Property<Slider, float> Value   { this, "Value",   0.0f, InvalidationKind::Visual };
		Property<Slider, float> Step    { this, "Step",    1.0f, InvalidationKind::None };

		Property<Slider, terminality::Orientation> Orientation
			{ this, "Orientation", terminality::Orientation::Horizontal, InvalidationKind::Measure };

		Event<float> ValueChanged;

		Slider() = default;

		/// Normalized position in [0,1].
		float Fraction() const;

		bool IsFocusable() const override { return true; }
		bool OnKeyDown(InputEvent input) override;
		void OnLostFocus() override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}
