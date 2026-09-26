#include "SearchBox.hpp"

bool btop::SearchBox::OnKeyDown(terminality::InputEvent input)
{
	if (input.Pressed && input.Key == terminality::InputKey::RETURN && !AcceptsReturn.Get())
	{
		Submitted.Emit();
		return true;
	}

	return TextBox::OnKeyDown(input);
}
