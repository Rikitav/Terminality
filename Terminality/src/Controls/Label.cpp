module terminality;

import std;
import std.compat;

using namespace terminality;

Label::Label()
{
	SetFocusable(false);
}

Label::Label(std::wstring& text)
{
	SetFocusable(false);
	Text = text;
}

void Label::OnPropertyChanged(const char* propertyName)
{
	if (std::strcmp(propertyName, "Text") == 0)
	{
		TextChanged.Emit();
	}

	ControlBase::OnPropertyChanged(propertyName);
}

Size Label::MeasureOverride(const Size& availableSize)
{
	std::vector<int32_t> lines = TextHelper::MeasureLines(Text, availableSize.Width, TextWrapping);

	int32_t maxWidth = 0;
	for (const auto& line : lines)
		maxWidth = std::max(maxWidth, line);

	int32_t width = availableSize.Width >= 0 ? std::clamp(maxWidth + 1, 0, availableSize.Width) : maxWidth + 1;
	int32_t desiredHeight = std::max<int32_t>(1, static_cast<int32_t>(lines.size()));
	int32_t height = availableSize.Height >= 0 ? std::min(desiredHeight, availableSize.Height) : desiredHeight;

	return Size(width, height);
}

void Label::ArrangeOverride(const Rect& contentRect)
{
	return;
}

void Label::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();

	Color fore = focused_ ? FocusedForegroundColor : ForegroundColor;
	Color back = focused_ ? FocusedBackgroundColor : BackgroundColor;

	std::vector<LineInfo> lines = TextHelper::GetLines(Text.Get(), rect.Width, TextWrapping.Get());
	for (const auto& line : lines)
		context.RenderText(Point::Zero, line.Text, fore, back, false);
}
