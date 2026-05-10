export module terminality:Layout;

export namespace terminality
{
	enum class HorizaontalAllign
	{
		Left,
		Center,
		Right,
		Stretch
	};

	enum class VerticalAlign
	{
		Top,
		Center,
		Bottom,
		Stretch
	};

	enum class TextWrap
	{
		NoWrap,
		Wrap,
		WrapWholeWords
	};

	enum class TextAlign
	{
		Left,
		Center,
		Right,
		Justify
	};

	enum class Orientation
	{
		Vertical,
		Horizontal
	};
}