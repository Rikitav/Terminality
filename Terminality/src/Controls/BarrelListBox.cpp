
#include <algorithm>
#include <cstring>

#include <terminality/Controls/BarrelListBox.hpp>

using namespace terminality;

BarrelListBox::BarrelListBox()
{
	itemAdded_ = Items.ItemAdded.Connect(
		[this](std::size_t index, const std::wstring& item) { OnItemAdded(index, item); });

	itemRemoved_ = Items.ItemRemoved.Connect(
		[this](std::size_t index, const std::wstring& item) { OnItemRemoved(index, item); });

	itemReplaced_ = Items.ItemReplaced.Connect(
		[this](std::size_t index, const std::wstring& oldItem, const std::wstring& newItem) { OnItemReplaced(index, oldItem, newItem); });

	collectionCleared_ = Items.CollectionCleared.Connect(
		[this]() { OnCollectionCleared(); });
}

const std::wstring* BarrelListBox::GetSelectedItem() const
{
	if (Items.empty())
		return nullptr;

	int32_t index = std::clamp(SelectedIndex.Get(), 0, static_cast<int32_t>(Items.size()) - 1);
	return &Items[static_cast<std::size_t>(index)];
}

void BarrelListBox::OnPropertyChanged(const char* propertyName)
{
	if (std::strcmp(propertyName, "SelectedIndex") == 0)
	{
		int32_t index = SelectedIndex.Get();

		if (Items.empty())
		{
			index = 0;
		}
		else
		{
			index = std::clamp(index, 0, static_cast<int32_t>(Items.size()) - 1);
		}

		if (index != SelectedIndex.Get())
		{
			// Write-back re-enters this handler once with the clamped value.
			SelectedIndex = index;
			return;
		}

		if (index != lastSelectedIndex_)
		{
			lastSelectedIndex_ = index;
			SelectionChanged.Emit(static_cast<std::size_t>(index));
		}
	}

	ControlBase::OnPropertyChanged(propertyName);
}

void BarrelListBox::EnsureSelectionValid()
{
	if (Items.empty())
	{
		SelectedIndex = 0;
		return;
	}

	SelectedIndex = std::clamp(SelectedIndex.Get(), 0, static_cast<int32_t>(Items.size()) - 1);
}

void BarrelListBox::OnItemAdded(std::size_t, const std::wstring&)
{
	InvalidateMeasure();
}

void BarrelListBox::OnItemRemoved(std::size_t, const std::wstring&)
{
	EnsureSelectionValid();
	InvalidateMeasure();
}

void BarrelListBox::OnItemReplaced(std::size_t, const std::wstring&, const std::wstring&)
{
	InvalidateMeasure();
}

void BarrelListBox::OnCollectionCleared()
{
	SelectedIndex = 0;
	InvalidateMeasure();
}

bool BarrelListBox::OnKeyDown(InputEvent input)
{
	if (!IsEnabled || Items.empty())
		return ControlBase::OnKeyDown(input);

	const int32_t count = static_cast<int32_t>(Items.size());
	const int32_t current = std::clamp(SelectedIndex.Get(), 0, count - 1);

	switch (input.Key)
	{
		case InputKey::UP:
		{
			int32_t next = current - 1;
			if (next < 0)
			{
				if (!Wrap.Get())
					return ControlBase::OnKeyDown(input); // at the top -> let focus leave
				next = count - 1;
			}
			SelectedIndex = next;
			return true;
		}

		case InputKey::DOWN:
		{
			int32_t next = current + 1;
			if (next >= count)
			{
				if (!Wrap.Get())
					return ControlBase::OnKeyDown(input); // at the bottom -> let focus leave
				next = 0;
			}
			SelectedIndex = next;
			return true;
		}
	}

	return ControlBase::OnKeyDown(input);
}

Size BarrelListBox::MeasureOverride(const Size& availableSize)
{
	EnsureSelectionValid();

	int32_t maxWidth = MaxSize.Get().Width;
	for (const std::wstring& item : Items)
		maxWidth = std::max(maxWidth, static_cast<int32_t>(item.size() + 2));

	int32_t width = (availableSize.Width >= 0) ? std::min(availableSize.Width, maxWidth) : maxWidth;

	int32_t rows = std::max(1, VisibleRows.Get());
	if (MinSize.Get().Height >= 0)
		rows = std::max(rows, MinSize.Get().Height);

	int32_t height = (availableSize.Height >= 0) ? std::min(availableSize.Height, rows) : rows;
	return Size(std::max(0, width), height);
}

void BarrelListBox::ArrangeOverride(const Rect&)
{
	// Single-cell control: nothing to arrange.
}

static wchar_t EmptyLineStyle(const Point& point, const Vector& vector)
{
	return L' ';
}

void BarrelListBox::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	if (rect.Width <= 0 || rect.Height <= 0 || Items.empty())
		return;

	const int32_t count = static_cast<int32_t>(Items.size());
	const int32_t selected = std::clamp(SelectedIndex.Get(), 0, count - 1);
	const int32_t centerRow = rect.Height / 2;
	const bool wrap = Wrap.Get();

	for (int32_t y = 0; y < rect.Height; ++y)
	{
		const int32_t offset = y - centerRow; // 0 == center row
		int32_t index = selected + offset;
		bool hasItem = true;

		if (wrap)
		{
			index = ((index % count) + count) % count;
		}
		else if (index < 0 || index >= count)
		{
			hasItem = false; // blank row outside the item range
			continue;
		}

		const std::wstring& item = Items[static_cast<std::size_t>(index)];
		const int32_t textLen = static_cast<int32_t>(item.length());
		const int32_t x = std::max(1, (std::max(0, rect.Width) - textLen) / 2);

		if (offset == 0) // is selected?
		{
			Color fore = FocusedForegroundColor.Get();
			Color back = FocusedBackgroundColor.Get();

			// Render text
			context.RenderLine(Point(0, y), rect.Width, EmptyLineStyle, fore, back);
			context.RenderText(Point(x, y), item, fore, back, false);

			// Render rails
			context.SetCell(0, y, L'\u251C', fore, back);
			context.SetCell(rect.Width - 1, y, L'\u2524', fore, back);
		}
		else
		{
			// Fade out the farther an option sits from the selected center row.
			const int32_t distance = (offset < 0) ? -offset : offset;
			const float tint = std::min(0.75f, 0.28f * static_cast<float>(distance));

			Color fore = Color::Darken(ForegroundColor.Get(), tint);
			Color back = BackgroundColor.Get();

			// Render text
			context.RenderLine(Point(0, y), rect.Width, EmptyLineStyle, fore, back);
			context.RenderText(Point(x, y), item, fore, back, false);

			// Render rails
			context.SetCell(0, y, L'\u2502', fore, back);
			context.SetCell(rect.Width - 1, y, L'\u2502', fore, back);
		}

	}

	/*
	context.RenderRectangle(
		Point(0, 0), rect.Width, rect.Height,
		ForegroundColor.Get(), BackgroundColor.Get(),
		[](const Point& point, const Size& size) { return L' '; });

	for (int32_t y = 0; y < rect.Height; ++y)
	{
		const int32_t offset = y - centerRow; // 0 == center row
		int32_t index = selected + offset;
		bool hasItem = true;

		if (wrap)
		{
			index = ((index % count) + count) % count;
		}
		else if (index < 0 || index >= count)
		{
			hasItem = false; // blank row outside the item range
		}

		const bool isSelected = (offset == 0);
		const int32_t distance = (offset < 0) ? -offset : offset;

		Color fg;
		Color bg;
		if (isSelected)
		{
			fg = FocusedForegroundColor.Get();
			bg = FocusedBackgroundColor.Get();
		}
		else
		{
			// Fade out the farther an option sits from the selected center row.
			const float tint = std::min(0.75f, 0.28f * static_cast<float>(distance));
			fg = Color::Darken(ForegroundColor.Get(), tint);
			bg = BackgroundColor.Get();
		}

		std::wstring line;
		line.assign(static_cast<std::size_t>(rect.Width), L' ');
		if (hasItem)
		{
			const std::wstring& item = Items[static_cast<std::size_t>(index)];
			const int32_t textWidth = std::clamp<int32_t>(static_cast<int32_t>(item.size()), 0, rect.Width) - 1;
			line.replace(0, static_cast<std::size_t>(textWidth), item, 0, static_cast<std::size_t>(textWidth));
		}

		context.SetCell(0, y, L'\u2502', fg, bg);
		context.RenderText(Point(1, y), line, fg, bg, false);
		context.SetCell(rect.Width - 1, y, L'\u2502', fg, bg);
	}
	*/
}
