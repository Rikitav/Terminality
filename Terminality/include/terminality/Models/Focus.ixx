export module terminality:Focus;

export namespace terminality
{
	enum class Direction
	{
		Up,
		Down,
		Left,
		Right,
		Next,	// Tab
		Previous // Shift + Tab
	};

	enum class NavigationAction
	{
		Moved,			// Focus successfully moved to the returned Widget
		PassToParent,	// I can't handle this, ask my parent
		Stop			// I can't handle this, but DO NOT pop
	};
}
