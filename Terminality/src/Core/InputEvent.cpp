
#include <cstdint>
#include <functional>

#include <terminality/Core/InputEvent.hpp>

using namespace terminality;

InputEvent::InputEvent(wchar_t ch, bool pressed)
	: Key(InputKey::CHAR), Char(ch), Pressed(pressed) { }

InputEvent::InputEvent(InputModifier modifier, InputKey key, bool pressed)
	: Modifier(modifier), Key(key), Pressed(pressed) { }

InputEvent::InputEvent(InputModifier modifier, InputKey key, wchar_t ch, bool pressed)
	: Modifier(modifier), Key(key), Char(ch), Pressed(pressed) { }

bool InputEvent::operator==(const InputEvent& other) const
{
	if (Modifier != other.Modifier || Key != other.Key || Pressed != other.Pressed)
		return false;

	// Character code is only meaningful for actual character events.
	if (Key == InputKey::CHAR)
		return Char == other.Char;

	return true;
}

std::size_t InputEventHasher::operator()(const InputEvent& e) const
{
	std::size_t hash =
		(std::hash<int>()(static_cast<int>(e.Modifier)) << 0) ^
		(std::hash<int>()(static_cast<int>(e.Key)) << 1) ^
		(std::hash<bool>()(e.Pressed) << 2);

	// Only hash the character for actual character events.
	if (e.Key == InputKey::CHAR)
		hash ^= std::hash<wchar_t>()(e.Char) << 3;

	return hash;
}
