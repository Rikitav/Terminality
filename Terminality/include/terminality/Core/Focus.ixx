export module terminality:Focus;

export namespace terminality
{
	enum class Direction
	{
		Up,
		Down,
		Left,
		Right,
		Next,	 // Tab
		Previous // Shift + Tab
	};
}
