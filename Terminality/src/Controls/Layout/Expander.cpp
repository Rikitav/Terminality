
#include <algorithm>
#include <cstring>
#include <string>

#include <terminality/Terminality.hpp>

using namespace terminality;

void Expander::Expand()
{
	if (Expanded.Get())
		return;

	Expanded = true;
	ExpandedChanged.Emit(true);
}

void Expander::Collapse()
{
	if (!Expanded.Get())
		return;

	Expanded = false;
	ExpandedChanged.Emit(false);
}

void Expander::Toggle()
{
	if (Expanded.Get())
		Collapse();
	else
		Expand();
}

void Expander::OnContentChanging(const std::unique_ptr<ControlBase>& oldContent)
{
	if (oldContent != nullptr)
		oldContent->SetParent(nullptr);
}

void Expander::OnPropertyChanged(const char* propertyName)
{
	if (std::strcmp(propertyName, "Content") == 0)
	{
		if (Content.Get() != nullptr)
			Content.Get()->SetParent(this);

		InvalidateMeasure();
		InvalidateVisual();
		return;
	}

	ControlBase::OnPropertyChanged(propertyName);
}

bool Expander::OnKeyDown(InputEvent input)
{
	switch (input.Key)
	{
		case InputKey::RETURN:
		case InputKey::SPACE:
			Toggle();
			return true;

		case InputKey::RIGHT:
			if (!Expanded.Get())
				Expand();
			else if (Content.Get() != nullptr && Content.Get()->IsFocusable())
				PushFocus(Content.Get().get());
			return true;

		case InputKey::LEFT:
			if (Expanded.Get())
				Collapse();
			return true;

		case InputKey::DOWN:
			if (Expanded.Get() && Content.Get() != nullptr && Content.Get()->IsFocusable())
			{
				PushFocus(Content.Get().get());
				return true;
			}
			break;

		default:
			break;
	}

	return ControlBase::OnKeyDown(input);
}

void Expander::OnGotFocus()
{
	focused_ = true;
	InvalidateVisual();
}

void Expander::OnLostFocus()
{
	focused_ = false;
	InvalidateVisual();
}

std::size_t Expander::VisualChildrenCount() const
{
	return (Expanded.Get() && Content.Get() != nullptr) ? 1 : 0;
}

VisualTreeNode* Expander::GetVisualChild(std::size_t index) const
{
	if (Expanded.Get() && Content.Get() != nullptr && index == 0)
		return Content.Get().get();

	return nullptr;
}

Size Expander::MeasureOverride(const Size& availableSize)
{
	int32_t width = 2 + static_cast<int32_t>(Header.Get().size()); // glyph + space + text
	int32_t height = 1;

	if (Expanded.Get() && Content.Get() != nullptr)
	{
		Size inner = availableSize;
		if (inner.Height >= 0)
			inner.Height = std::max(0, inner.Height - 1);

		Size contentSize = Content.Get()->Measure(inner);

		if (contentSize.Width > width)
			width = contentSize.Width;

		height += (contentSize.Height < 0 ? 0 : contentSize.Height);
	}

	if (availableSize.Width >= 0)
		width = std::min(width, availableSize.Width);

	if (availableSize.Height >= 0)
		height = std::min(height, availableSize.Height);

	return Size(width, height);
}

void Expander::ArrangeOverride(const Rect& contentRect)
{
	if (!Expanded.Get() || Content.Get() == nullptr)
		return;

	Rect inner(
		contentRect.X,
		contentRect.Y + 1,
		contentRect.Width,
		std::max(0, contentRect.Height - 1));

	Content.Get()->Arrange(inner);
}

void Expander::RenderOverride(RenderContext& context)
{
	std::wstring line;
	line.push_back(Expanded.Get() ? ExpandedGlyph.Get() : CollapsedGlyph.Get());
	line.push_back(L' ');
	line += Header.Get();

	Color fg = focused_ ? FocusedForegroundColor.Get() : ForegroundColor.Get();
	Color bg = focused_ ? FocusedBackgroundColor.Get() : BackgroundColor.Get();

	context.RenderText(Point::Zero, line, fg, bg, false);

	if (Expanded.Get() && Content.Get() != nullptr)
	{
		Rect childRect = Content.Get()->GetArrangedRect();
		RenderContext childContext = context.CreateInner(childRect);
		Content.Get()->Render(childContext);
	}
}
