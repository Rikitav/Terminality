export module terminality:Layout;

export namespace terminality
{
	enum class HorizontalAlignment
	{
		Left,
		Center,
		Right,
		Stretch
	};

	enum class VerticalAlignment
	{
		Top,
		Center,
		Bottom,
		Stretch
	};

	enum class TextWrapping
	{
		NoWrap,
		Wrap,
		WrapWholeWords
	};

	enum class TextAlignment
	{
		Left,
		Center,
		Right,
		Justify
	};
}