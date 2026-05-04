module terminality;

import std;
import std.compat;

using namespace terminality;

void Button::SetText(std::wstring text)
{
	text_ = std::move(text);
	InvalidateMeasure();
}

void Button::Click()
{
	Clicked.Emit();
	InvalidateVisual();
}

Size Button::MeasureOverride(const Size& availableSize)
{
	int32_t contentWidth = static_cast<int32_t>(text_.size()) + 6;
	int32_t width = availableSize.Width >= 0 ? std::min(availableSize.Width, contentWidth) : contentWidth;
	int32_t height = availableSize.Height >= 0 ? std::min(availableSize.Height, 1) : 1;
	return Size(width, height);
}

void Button::ArrangeOverride(const Rect& contentRect)
{
	return;
}

void Button::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	std::wstring line = L"[  " + text_ + L"  ]";

	Point point = Point(0, 0);
	Color fore = focused_ ? Color::BLACK : Color::WHITE;
	Color back = focused_ ? Color::WHITE : Color::BLACK;
	context.RenderText(point, line, fore, back, false);
	visualDirty_ = false;
}
