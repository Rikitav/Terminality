
#include <algorithm>
#include <vector>
#include <string>
#include <memory>
#include <cstring>

#include <terminality/Terminality.hpp>

using namespace terminality;

static constexpr bool in_bounds(int index, const std::vector<TabItem>& tabs)
{
	return index >= 0 && index < tabs.size();
}

static std::wstring ToWString(const std::string& text)
{
	return std::wstring(text.begin(), text.end());
}

static int32_t HeaderWidth(const TabItem& tab)
{
	std::wstring header = ToWString(tab.Header);
	int32_t width = 1 + static_cast<int32_t>(header.length()); // trailing space + header
	if (tab.IsCloseable)
		width += 4; // L"[x] "
	return width;
}

void TabControl::AddTab(const std::string& header, std::unique_ptr<ControlBase> content)
{
	AddTab(header, std::move(content), false);
}

void TabControl::AddTab(const std::string& header, std::unique_ptr<ControlBase> content, bool closeable)
{
	if (content)
	{
		content->SetParent(this);
		if (!content->IsAttached())
			content->OnAttachedToTree();
	}

	tabs_.emplace_back(header, std::move(content), closeable);
	InvalidateMeasure();
}

void TabControl::RemoveTab(int index)
{
	if (index >= 0 && index < static_cast<int>(tabs_.size()))
	{
		if (tabs_[index].Content)
			tabs_[index].Content->SetParent(nullptr);

		tabs_.erase(tabs_.begin() + index);
		TabClosed.Emit(index);

		if (SelectedIndex.Get() >= static_cast<int>(tabs_.size()))
			SelectedIndex = std::max(0, static_cast<int>(tabs_.size()) - 1);

		InvalidateMeasure();
	}
}

void TabControl::ClearTabs()
{
	for (auto& tab : tabs_)
	{
		if (tab.Content)
			tab.Content->SetParent(nullptr);
	}

	tabs_.clear();
	SelectedIndex = 0;
	InvalidateMeasure();
}

size_t TabControl::GetTabCount() const
{
	return tabs_.size();
}

bool TabControl::OnKeyDown(InputEvent input)
{
	if (!IsEnabled)
		return ControlBase::OnKeyDown(input);

	if (tabs_.empty())
		return false;

	bool ctrl = terminality::hasFlag(input.Modifier, InputModifier::Ctrl);
	if (ctrl && input.Key == InputKey::TAB)
	{
		int idx = SelectedIndex.Get();
		bool shift = terminality::hasFlag(input.Modifier, InputModifier::Shift);
		if (shift)
			idx = (idx <= 0) ? static_cast<int>(tabs_.size()) - 1 : idx - 1;
		else
			idx = (idx + 1 >= static_cast<int>(tabs_.size())) ? 0 : idx + 1;

		SelectedIndex = idx;
		return true;
	}

	if (input.Key == InputKey::LEFT)
	{
		int idx = SelectedIndex.Get();
		if (idx > 0)
		{
			SelectedIndex = idx - 1;
			return true;
		}
	}
	else if (input.Key == InputKey::RIGHT)
	{
		int idx = SelectedIndex.Get();
		if (idx < static_cast<int>(tabs_.size()) - 1)
		{
			SelectedIndex = idx + 1;
			return true;
		}
	}
	else if (input.Key == InputKey::DELETE)
	{
		int idx = SelectedIndex.Get();
		if (in_bounds(idx, tabs_) && tabs_[idx].IsCloseable)
		{
			RemoveTab(idx);
			return true;
		}
	}
	else if (input.Key == InputKey::DOWN || input.Key == InputKey::RETURN)
	{
		int idx = SelectedIndex.Get();
		if (in_bounds(idx, tabs_))
		{
			ControlBase* content = tabs_[idx].Content.get();
			if (content != nullptr && content->IsFocusable())
			{
				PushFocus(content);
				return true;
			}
		}
	}

	return ControlBase::OnKeyDown(input);
}

std::size_t TabControl::VisualChildrenCount() const
{
	int idx = SelectedIndex.Get();
	if (in_bounds(idx, tabs_) && tabs_[idx].Content != nullptr)
		return 1;

	return 0;
}

VisualTreeNode* TabControl::GetVisualChild(std::size_t index) const
{
	int idx = SelectedIndex.Get();
	if (in_bounds(idx, tabs_))
	{
		ControlBase* content = tabs_[idx].Content.get();
		if (content != nullptr && index == 0)
			return content;
	}

	return nullptr;
}

bool TabControl::MoveFocusNext(Direction direction, InputModifier modifiers)
{
	if (tabs_.empty())
		return false;

	int idx = SelectedIndex.Get();
	if (in_bounds(idx, tabs_))
	{
		if (direction == Direction::Down || direction == Direction::Next)
		{
			ControlBase* content = tabs_[idx].Content.get();
			if (content != nullptr && content->IsFocusable())
			{
				PushFocus(content);
				return true;
			}
		}
	}

	return false;
}

void TabControl::OnGotFocus()
{
	InvalidateVisual();
}

void TabControl::OnLostFocus()
{
	InvalidateVisual();
}

void TabControl::OnPropertyChanged(const char* propertyName)
{
	ControlBase::OnPropertyChanged(propertyName);
}

Size TabControl::MeasureOverride(const Size& availableSize)
{
	int32_t totalHeaderWidth = 0;
	for (const auto& tab : tabs_)
	{
		totalHeaderWidth += HeaderWidth(tab);
	}

	if (!tabs_.empty())
		totalHeaderWidth += static_cast<int32_t>(tabs_.size() - 1); // vertical separators

	Size contentAvailable = availableSize;
	if (contentAvailable.Height != -1)
		contentAvailable.Height = std::max(0, contentAvailable.Height - 2);

	int32_t maxContentWidth = 0;
	int32_t maxContentHeight = 0;

	int idx = SelectedIndex.Get();
	if (in_bounds(idx, tabs_))
	{
		ControlBase* content = tabs_[idx].Content.get();
		if (content != nullptr)
		{
			Size childSize = content->Measure(contentAvailable);
			maxContentWidth = childSize.Width;
			maxContentHeight = childSize.Height;
		}
	}

	return Size(
		std::max(totalHeaderWidth, maxContentWidth),
		maxContentHeight + 2
	);
}

void TabControl::ArrangeOverride(const Rect& finalRect)
{
	int idx = SelectedIndex.Get();
	if (in_bounds(idx, tabs_))
	{
		ControlBase* content = tabs_[idx].Content.get();
		if (content != nullptr)
		{
			Rect contentRect(
				finalRect.X,
				finalRect.Y + 2,
				finalRect.Width,
				std::max(0, finalRect.Height - 2));

			content->Arrange(contentRect);
		}
	}
}

static wchar_t DefaultUnderlineStyle(const Point& point, const Vector& vector)
{
	return L'\u2500';
}

static wchar_t FocusedUnderlineStyle(const Point& point, const Vector& vector)
{
	return L' ';
}

void TabControl::RenderOverride(RenderContext& context)
{
	int idx = SelectedIndex.Get();
	Rect bounds = context.ContextRect();

	Color defaultFg = ForegroundColor.Get();
	Color defaultBg = BackgroundColor.Get();

	int32_t currentX = 0;
	int32_t selectedStart = -1;
	int32_t selectedEnd = -1;

	for (int i = 0; i < static_cast<int>(tabs_.size()); ++i)
	{
		const auto& tab = tabs_[i];
		std::wstring header = ToWString(tab.Header);

		std::wstring body = header + L" ";
		if (tab.IsCloseable)
			body += L"[x] ";

		int32_t bodyWidth = static_cast<int32_t>(body.length());

		bool drawSeparator = (i + 1 < static_cast<int>(tabs_.size()));
		int width = bodyWidth + (drawSeparator ? 1 : 0);

		Color fg = (i == idx)
			? FocusedForegroundColor.Get()
			: ForegroundColor.Get();

		Color bg = (i == idx)
			? FocusedBackgroundColor.Get()
			: BackgroundColor.Get();

		if (currentX + width > bounds.Width)
		{
			int available = std::max(0, bounds.Width - currentX);
			int bodyAvailable = std::min(available, bodyWidth);
			if (bodyAvailable > 0)
				context.RenderText(Point(currentX, 0), body.substr(0, bodyAvailable), fg, bg);

			if (drawSeparator && available > bodyWidth)
				context.RenderText(Point(currentX + bodyWidth, 0), L"\u2502", defaultFg, defaultBg);

			break;
		}

		context.RenderText(Point(currentX, 0), body, fg, bg);

		if (drawSeparator)
			context.RenderText(Point(currentX + bodyWidth, 0), L"\u2502", defaultFg, defaultBg);

		if (i == idx)
		{
			selectedStart = currentX;
			selectedEnd = currentX + bodyWidth - 1;
		}

		currentX += width;
	}

	// Separator line between the tab strip and the content. Leave a gap under
	// the selected tab so it appears attached to its content.
	if (bounds.Height > 1)
	{
		Color lineFg = Color::DARK_GRAY;
		if (selectedStart > 0)
			context.RenderLine(Point(0, 1), selectedStart, DefaultUnderlineStyle, defaultFg, defaultBg);

		context.RenderLine(Point(selectedStart, 1), selectedEnd - selectedStart + 1, FocusedUnderlineStyle, FocusedForegroundColor.Get(), FocusedBackgroundColor.Get());
		context.RenderLine(Point(selectedEnd + 1, 1), bounds.Width - selectedEnd, DefaultUnderlineStyle, defaultFg, defaultBg);
	}

	if (in_bounds(idx, tabs_))
	{
		ControlBase* content = tabs_[idx].Content.get();
		if (content != nullptr)
		{
			Rect childRect = tabs_[idx].Content->GetArrangedRect();
			RenderContext childContext = context.CreateInner(childRect);
			tabs_[idx].Content->Render(childContext);
		}
	}
}
