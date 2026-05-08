module terminality;

import std;
import std.compat;
import :ContextMenu;

using namespace terminality;

void ControlBase::SetFocusable(bool value)
{
	if (focusable_ == value)
		return;

	focusable_ = value;
	OnPropertyChanged("Focusable");
}

void ControlBase::SetTabStop(bool value)
{
	if (isTabStop_ == value)
		return;

	isTabStop_ = value;
	OnPropertyChanged("TabStop");
}

void ControlBase::SetTabIndex(int value)
{
	if (tabIndex_ == value)
		return;

	tabIndex_ = value;
	OnPropertyChanged("TabIndex");
}

Size ControlBase::Measure(const Size& availableSize)
{
	int32_t innerWidth  = availableSize.Width;
	int32_t innerHeight = availableSize.Height;

	if (MaxSize->Width >= 0)
		innerWidth  = innerWidth  < 0 ? MaxSize->Width  : std::min(innerWidth,  MaxSize->Width);

	if (MaxSize->Height >= 0)
		innerHeight = innerHeight < 0 ? MaxSize->Height : std::min(innerHeight, MaxSize->Height);

	if (innerWidth >= 0)
		innerWidth  = std::max(0, innerWidth  - Margin->Left - Margin->Right);

	if (innerHeight >= 0)
		innerHeight = std::max(0, innerHeight - Margin->Top  - Margin->Bottom);

	if (MinSize->Width  >= 0 && innerWidth  >= 0)
		innerWidth  = std::max(innerWidth,  MinSize->Width);

	if (MinSize->Height >= 0 && innerHeight >= 0)
		innerHeight = std::max(innerHeight, MinSize->Height);

	Size innerSize(innerWidth, innerHeight);
	if (innerSize == Size::Zero)
	{
		actualSize_ = Size(Margin->Left + Margin->Right, Margin->Top + Margin->Bottom);
		return actualSize_;
	}

	Size contentSize = MeasureOverride(innerSize);
	actualSize_ = Size(
		contentSize.Width  >= 0 ? contentSize.Width  + Margin->Left + Margin->Right  : -1,
		contentSize.Height >= 0 ? contentSize.Height + Margin->Top  + Margin->Bottom : -1
	);

	return actualSize_;
}

void ControlBase::Arrange(const Rect& finalRect)
{
	int32_t slotX      = finalRect.X + Margin->Left;
	int32_t slotY      = finalRect.Y + Margin->Top;
	int32_t slotWidth  = std::max(0, finalRect.Width  - Margin->Left - Margin->Right);
	int32_t slotHeight = std::max(0, finalRect.Height - Margin->Top  - Margin->Bottom);

	int32_t contentWidth  = actualSize_.Width  >= 0 ? std::max(0, actualSize_.Width  - Margin->Left - Margin->Right)  : -1;
	int32_t contentHeight = actualSize_.Height >= 0 ? std::max(0, actualSize_.Height - Margin->Top  - Margin->Bottom) : -1;

	int32_t x      = slotX;
	int32_t y      = slotY;
	int32_t width  = contentWidth;
	int32_t height = contentHeight;

	switch (HorizontalAlignment)
	{
		case HorizontalAlignment::Left:
		{
			width = width < 0 ? slotWidth : std::min(width, slotWidth);
			break;
		}

		case HorizontalAlignment::Center:
		{
			width = width < 0 ? slotWidth : std::min(width, slotWidth);
			x += (slotWidth - width) / 2;
			break;
		}

		case HorizontalAlignment::Right:
		{
			width = width < 0 ? slotWidth : std::min(width, slotWidth);
			x += slotWidth - width;
			break;
		}

		case HorizontalAlignment::Stretch:
		{
			width = std::clamp<int32_t>(slotWidth,
				MinSize->Width  < 0 ? 0         : MinSize->Width,
				MaxSize->Width  < 0 ? slotWidth : MaxSize->Width);

			x += (slotWidth - width) / 2;
			break;
		}
	}

	switch (VerticalAlignment)
	{
		case VerticalAlignment::Top:
		{
			height = height < 0 ? slotHeight : std::min(height, slotHeight);
			break;
		}

		case VerticalAlignment::Center:
		{
			height = height < 0 ? slotHeight : std::min(height, slotHeight);
			y += (slotHeight - height) / 2;
			break;
		}

		case VerticalAlignment::Bottom:
		{
			height = height < 0 ? slotHeight : std::min(height, slotHeight);
			y += slotHeight - height;
			break;
		}

		case VerticalAlignment::Stretch:
		{
			height = std::clamp<int32_t>(slotHeight,
				MinSize->Height < 0 ? 0          : MinSize->Height,
				MaxSize->Height < 0 ? slotHeight : MaxSize->Height);

			y += (slotHeight - height) / 2;
			break;
		}
	}

	arrangeDirty_ = false;
	arrangedRect_ = Rect(x, y, width, height);
	ArrangeOverride(arrangedRect_);
}

static wchar_t EmptyRectangleStyle(const Point& point, const Size& size)
{
	return L' ';
}

void ControlBase::Render(RenderContext& context)
{
	Rect rect = context.ContextRect();
	context.RenderRectangle(Point::Zero, rect.Size(), ForegroundColor, BackgroundColor, EmptyRectangleStyle);

	visualDirty_ = false;
	RenderOverride(context);
}

void ControlBase::SetParent(VisualTreeNode* parent, UILayer* layer)
{
	if (parent_ == parent)
		return;

	if (parent == nullptr)
	{
		OnDettachedFromTree();
		parent_ = nullptr;
		layer_ = nullptr;
		return;
	}

	parent_ = parent;
	layer_ = layer;
	OnPropertyChanged("Parent");
}

const std::span<VisualTreeNode*> ControlBase::GetChildren() const
{
	static std::span<VisualTreeNode*> dummy;
	return dummy;
}

void ControlBase::OpenContextMenu()
{
	if (CtxMenu.Get() != nullptr)
	{
		Rect rect = GetArrangedRect();
		CtxMenu.Get()->Open(Point(rect.X, rect.Y + rect.Height));
	}
}

void ControlBase::ApplyInvalidation(InvalidationKind invalidation)
{
	if (hasFlag(invalidation, InvalidationKind::Visual))
		InvalidateVisual();

	if (hasFlag(invalidation, InvalidationKind::Arrange))
		InvalidateArrange();

	if (hasFlag(invalidation, InvalidationKind::Measure))
		InvalidateMeasure();
}

bool ControlBase::OnKeyDown(InputEvent input)
{
	KeyDown.Emit(input);
	
	for (const auto& pair : hotkeys_)
	{
		const InputEvent event = pair.first;
		if (input.Modifier == event.Modifier && input.Key == event.Key)
		{
			KeyCombinationCallback callback = pair.second;
			callback(this);
			return true;
		}
	}

	switch (input.Key)
	{
		case InputKey::None:
		{
			break;
		}

		case InputKey::UP:
		{
			PopFocus(Direction::Up, input.Modifier);
			return true;
		}

		case InputKey::DOWN:
		{
			PopFocus(Direction::Down, input.Modifier);
			return true;
		}

		case InputKey::LEFT:
		{
			PopFocus(Direction::Left, input.Modifier);
			return true;
		}

		case InputKey::RIGHT:
		{
			PopFocus(Direction::Right, input.Modifier);
			return true;
		}

		case InputKey::TAB:
		{
			PopFocus(hasFlag(input.Modifier, InputModifier::Shift) ? Direction::Previous : Direction::Next, input.Modifier);
			return true;
		}
	}

	return false;
}

bool ControlBase::OnKeyUp(InputEvent input)
{
	KeyUp.Emit(input);
	return false;
}

void ControlBase::OnHotkey(InputModifier modifier, InputKey key, KeyCombinationCallback callback)
{
	InputEvent event(modifier, key, true);
	if (hotkeys_.find(event) != hotkeys_.end())
		throw std::runtime_error("Combination is already registered.");

	hotkeys_[event] = std::move(callback);
}

void ControlBase::OnPropertyChanged(const char* propertyName)
{
	if (std::strcmp(propertyName, "ExpSize") == 0)
	{
		Size newSize = ExpSize.Get();
		MinSize.Set(std::move(newSize));
		MaxSize.Set(std::move(newSize));
	}
}
