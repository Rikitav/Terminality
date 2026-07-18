
#include <cstdint>
#include <string>
#include <algorithm>
#include <cstring>

#include <terminality/Terminality.hpp>

using namespace terminality;

Label::Label()
{
	SetFocusable(false);
}

Label::Label(const std::wstring& text)
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
	const Thickness padding = Padding.Get();
	int32_t padHoriz = padding.Horizontal();
	int32_t padVert = padding.Vertical();

	int32_t innerWidth = availableSize.Width >= 0 ? std::max(0, availableSize.Width - padHoriz) : -1;
	std::vector<int32_t> lines = TextHelper::MeasureLines(Text, innerWidth, TextWrapping);

	int32_t maxWidth = 0;
	for (const auto& line : lines)
		maxWidth = std::max(maxWidth, line);

	int32_t width = availableSize.Width >= 0
		? std::clamp(maxWidth + 1 + padHoriz, 0, availableSize.Width)
		: maxWidth + 1 + padHoriz;

	int32_t desiredHeight = std::max<int32_t>(1, static_cast<int32_t>(lines.size())) + padVert;
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
	const Thickness padding = Padding.Get();

	Color fore = GetEffectiveForegroundColor();
	Color back = GetEffectiveBackgroundColor();

	int32_t innerWidth = std::max(0, rect.Width - padding.Horizontal());
	std::vector<LineInfo> lines = TextHelper::GetLines(Text.Get(), innerWidth, TextWrapping.Get());

	for (std::size_t i = 0; i < lines.size(); ++i)
	{
		int32_t y = padding.Top + static_cast<int32_t>(i);
		if (y >= rect.Height - padding.Bottom)
			break;

		int32_t textLen = static_cast<int32_t>(lines[i].Text.length());
		int32_t x = padding.Left;

		switch (TextAlignment.Get())
		{
			case TextAlign::Center:
				x += std::max(0, (innerWidth - textLen) / 2);
				break;

			case TextAlign::Right:
				x += std::max(0, innerWidth - textLen);
				break;

			case TextAlign::Left:
			default:
				break;
		}

		context.RenderText(Point(x, y), lines[i].Text, fore, back, false);
	}
}
