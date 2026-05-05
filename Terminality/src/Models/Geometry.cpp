module terminality;

import std;
import std.compat;

using namespace terminality;

const Point Point::Zero = { 0, 0 };

bool Point::operator==(const Point& other) const
{
	return X == other.X && Y == other.Y;
}

bool Point::operator!=(const Point& other) const
{
	return !(*this == other);
}

bool Vector::operator==(const Vector& other) const
{
	return From == other.From && To == other.To;
}

bool Vector::operator!=(const Vector& other) const
{
	return !(*this == other);
}

const Size Size::Zero = { 0, 0 };
const Size Size::Auto = { -1, -1 };

bool Size::operator==(const Size& other) const
{
	return Width == other.Width && Height == other.Height;
}

bool Size::operator!=(const Size& other) const
{
	return !(*this == other);
}

const Thickness Thickness::Zero = { 0, 0, 0, 0 };
const Thickness Thickness::Single = { 1, 1, 1, 1 };

bool Thickness::operator==(const Thickness& other) const
{
	return Left == other.Left && Top == other.Top && Right == other.Right && Bottom == other.Bottom;
}

bool Thickness::operator!=(const Thickness& other) const
{
	return !(*this == other);
}

bool Thickness::IsUniform() const
{
	return Left == Top && Left == Right && Left == Bottom;
}

bool Rect::operator==(const Rect& other) const
{
	return X == other.X && Y == other.Y && Width == other.Width && Height == other.Height;
}

bool Rect::operator!=(const Rect& other) const
{
	return !(*this == other);
}

int32_t Rect::Right() const
{
	return X + Width;
}

int32_t Rect::Bottom() const
{
	return Y + Height;
}

Size Rect::Size() const
{
	return terminality::Size(Width, Height);
}

bool Rect::IsEmpty() const
{
	return Width <= 0 || Height <= 0;
}

Rect Rect::Union(const Rect& a, const Rect& b)
{
	if (a.IsEmpty())
		return b;

	if (b.IsEmpty())
		return a;

	const int32_t left = std::min(a.X, b.X);
	const int32_t top = std::min(a.Y, b.Y);

	const int32_t right = std::max(a.Right(), b.Right());
	const int32_t bottom = std::max(a.Bottom(), b.Bottom());

	return Rect(left, top, right - left, bottom - top);
}

Rect Rect::Enclose(const Rect& into, const Rect& rect)
{
	const int32_t absLeft = std::max(into.X, into.X + rect.X);
	const int32_t absTop = std::max(into.Y, into.Y + rect.Y);

	const int32_t rectRight = into.X + rect.X + rect.Width;
	const int32_t rectBottom = into.Y + rect.Y + rect.Height;

	const int32_t absRight = std::min(into.X + into.Width, rectRight);
	const int32_t absBottom = std::min(into.Y + into.Height, rectBottom);

	const int32_t finalWidth = std::max(0, absRight - absLeft);
	const int32_t finalHeight = std::max(0, absBottom - absTop);

	return Rect(absLeft, absTop, finalWidth, finalHeight);
}

Rect Rect::Clip(const Rect& into, const Rect& rect)
{
	const int32_t left = std::max(into.X, rect.X);
	const int32_t top = std::max(into.Y, rect.Y);

	const int32_t right = std::min(into.X + into.Width, rect.X + rect.Width);
	const int32_t bottom = std::min(into.Y + into.Height, rect.Y + rect.Height);

	const int32_t width = std::max(0, right - left);
	const int32_t height = std::max(0, bottom - top);

	return Rect(left, top, width, height);
}
