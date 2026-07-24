
#include <cstdint>
#include <algorithm>
#include <optional>
#include <string>
#include <cstring>

#include <terminality/Controls/CheckBox.hpp>

using namespace terminality;

void CheckBox::Toggle(std::optional<bool> value)
{
	if (IsChecked == value)
		return;

	IsChecked = value;
}

void CheckBox::OnPropertyChanged(const char* propertyName)
{
	if (std::strcmp(propertyName, "IsChecked") == 0)
	{
		Toggled.Emit(IsChecked.Get());
		InvalidateVisual();

		if (IsChecked.Get().has_value())
		{
			if (IsChecked.Get().value())
				Checked.Emit();
			else
				Unchecked.Emit();
		}
	}

	ControlBase::OnPropertyChanged(propertyName);
}

bool CheckBox::OnKeyDown(InputEvent input)
{
	if (!IsEnabled)
		return ControlBase::OnKeyDown(input);

	switch (input.Key)
	{
		case InputKey::RETURN:
		case InputKey::SPACE:
		{
			isPressed_ = true;
			InvalidateVisual();
			return true;
		}
	}

	return ControlBase::OnKeyDown(input);
}

bool CheckBox::OnKeyUp(InputEvent input)
{
	if (!IsEnabled)
		return ControlBase::OnKeyUp(input);

	switch (input.Key)
	{
		case InputKey::RETURN:
		case InputKey::SPACE:
		{
			isPressed_ = false;

			std::optional<bool> current = IsChecked.Get();
			if (!current.has_value())
			{
				Toggle(true);
			}
			else if (IsThreeState.Get())
			{
				if (current.value())
					Toggle(false);
				else
					Toggle(std::nullopt);
			}
			else
			{
				Toggle(!current.value());
			}

			return true;
		}
	}

	return ControlBase::OnKeyUp(input);
}

void CheckBox::OnLostFocus()
{
	focused_ = false;
	isPressed_ = false;
	InvalidateVisual();
}

Size CheckBox::MeasureOverride(const Size& availableSize)
{
	int32_t contentWidth = static_cast<int32_t>(Text->size()) + 7 + 3;
	int32_t width = availableSize.Width >= 0 ? std::min(availableSize.Width, contentWidth) : contentWidth;
	int32_t height = availableSize.Height >= 0 ? std::min(availableSize.Height, 1) : 1;
	return Size(width, height);
}

void CheckBox::ArrangeOverride(const Rect& contentRect)
{
	// bleh U_U
	return;
}

void CheckBox::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	std::wstring line = L"";

	std::optional<bool> checked = IsChecked.Get();
	if (!checked.has_value())
	{
		line += L"[?]";
	}
	else if (checked.value())
	{
		line += L"[X]";
	}
	else
	{
		line += L"[ ]";
	}

	line += L" [  " + Text.Get() + L"  ]";

	Color fore = GetEffectiveForegroundColor();
	Color back = GetEffectiveBackgroundColor();

	if (IsEnabled && focused_)
	{
		if (isPressed_)
		{
			fore = PressedForegroundColor;
			back = PressedBackgroundColor;
		}
		else
		{
			fore = FocusedForegroundColor;
			back = FocusedBackgroundColor;
		}
	}

	context.RenderText(Point::Zero, line, fore, back, false);
}
