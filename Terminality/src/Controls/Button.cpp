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

Size Button::Measure(const Size& availableSize)
{
	Size desiredSize = ControlBase::Measure(availableSize);
	const int32_t width = std::min<int32_t>(desiredSize.Width, static_cast<int32_t>(text_.size()) + 6);
	
	actualSize_ = Size(width, 1);
	return actualSize_;
}

void Button::Render(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	std::wstring line = L"[  " + text_ + L"  ]";

	Point point = Point(0, 0);
	Color fore = focused_ ? Color::BLACK : Color::WHITE;
	Color back = focused_ ? Color::WHITE : Color::BLACK;
	context.RenderText(point, line, fore, back, false);
	visualDirty_ = false;
}
