export module terminality:ProgressBar;

import :Event;
import :Color;
import :ControlBase;
import :Layout;
import :RenderContext;
import :Property;

export namespace terminality
{
	class ProgressBar : public ControlBase
	{
	public:
		Property<ProgressBar, float> Minimum { this, "Minimum", 0.0f, InvalidationKind::Visual };
		Property<ProgressBar, float> Maximum { this, "Maximum", 100.0f, InvalidationKind::Visual };
		Property<ProgressBar, float> Value   { this, "Value", 0.0f, InvalidationKind::Visual };

		Property<ProgressBar, Color> BarColor   { this, "BarColor", Color::GREEN, InvalidationKind::Visual };
		Property<ProgressBar, Color> TrackColor { this, "TrackColor", Color::DARK_GRAY, InvalidationKind::Visual };

		ProgressBar() = default;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}
