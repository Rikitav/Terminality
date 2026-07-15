
#include <terminality/Terminality.hpp>

using namespace terminality;

float Slider::Fraction() const
{
	float min = Minimum.Get();
	float max = Maximum.Get();

	if (max <= min)
		return 0.0f;

	float f = (Value.Get() - min) / (max - min);

	if (f < 0.0f) f = 0.0f;
	if (f > 1.0f) f = 1.0f;

	return f;
}

bool Slider::OnKeyDown(InputEvent input)
{
	const float step = Step.Get();
	float v = Value.Get();
	const bool horizontal = (Orientation.Get() == terminality::Orientation::Horizontal);

	if (horizontal && input.Key == InputKey::LEFT)        v -= step;
	else if (horizontal && input.Key == InputKey::RIGHT)  v += step;
	else if (!horizontal && input.Key == InputKey::DOWN)  v -= step;
	else if (!horizontal && input.Key == InputKey::UP)    v += step;
	else
		return ControlBase::OnKeyDown(input);

	float min = Minimum.Get();
	float max = Maximum.Get();

	if (v < min) v = min;
	if (v > max) v = max;

	Value = v;
	ValueChanged.Emit(v);
	return true;
}

void Slider::OnLostFocus()
{
	focused_ = false;
	InvalidateVisual();
}

Size Slider::MeasureOverride(const Size& availableSize)
{
	const bool horizontal = (Orientation.Get() == terminality::Orientation::Horizontal);

	if (horizontal)
	{
		int32_t width = availableSize.Width >= 0 ? availableSize.Width : 10;
		int32_t height = availableSize.Height >= 0 ? std::min(1, availableSize.Height) : 1;
		return Size(width, height);
	}

	int32_t height = availableSize.Height >= 0 ? availableSize.Height : 10;
	int32_t width = availableSize.Width >= 0 ? std::min(1, availableSize.Width) : 1;
	return Size(width, height);
}

void Slider::ArrangeOverride(const Rect& /*contentRect*/)
{
}

void Slider::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();

	Color activeFg = focused_ ? FocusedForegroundColor.Get() : ForegroundColor.Get();
	Color activeBg = focused_ ? FocusedBackgroundColor.Get() : BackgroundColor.Get();
	Color dimFg = Color::DARK_GRAY;

	if (Orientation.Get() == terminality::Orientation::Horizontal)
	{
		if (rect.Width <= 0)
			return;

		int32_t handleX = static_cast<int32_t>(Fraction() * (rect.Width - 1) + 0.5f);

		for (int32_t x = 0; x < rect.Width; ++x)
		{
			if (x == handleX)
				context.SetCell(x, 0, L'#', activeFg, activeBg);
			else if (x < handleX)
				context.SetCell(x, 0, L'=', activeFg, activeBg);
			else
				context.SetCell(x, 0, L'-', dimFg, activeBg);
		}
	}
	else
	{
		if (rect.Height <= 0)
			return;

		int32_t handleY = static_cast<int32_t>(Fraction() * (rect.Height - 1) + 0.5f);

		for (int32_t y = 0; y < rect.Height; ++y)
		{
			if (y == handleY)
				context.SetCell(0, y, L'#', activeFg, activeBg);
			else if (y < handleY)
				context.SetCell(0, y, L'|', activeFg, activeBg);
			else
				context.SetCell(0, y, L'.', dimFg, activeBg);
		}
	}
}
