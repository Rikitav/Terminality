export module terminality:ProgressBar;

import std;
import :ControlBase;
import :Layout;
import :RenderContext;
import :PropertyDescriptor;

export namespace terminality
{
	class ProgressBar : public ControlBase
	{
	public:
		PropertyDescriptor<ProgressBar, float> Minimum { this, "Minimum", 0.0f, InvalidationKind::Visual };
		PropertyDescriptor<ProgressBar, float> Maximum { this, "Maximum", 100.0f, InvalidationKind::Visual };
		PropertyDescriptor<ProgressBar, float> Value   { this, "Value", 0.0f, InvalidationKind::Visual };

		PropertyDescriptor<ProgressBar, Color> BarColor   { this, "BarColor", Color::GREEN, InvalidationKind::Visual };
		PropertyDescriptor<ProgressBar, Color> TrackColor { this, "TrackColor", Color::DARK_GRAY, InvalidationKind::Visual };

		ProgressBar() = default;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}