#pragma once

#include <cstdint>
#include <algorithm>

namespace terminality
{
	struct Point
	{
		static const Point Zero;

		int32_t X;
		int32_t Y;

		Point(int32_t x = 0, int32_t y = 0);

		bool operator==(const Point& other) const;
		bool operator!=(const Point& other) const;
	};

	struct Vector
	{
		Point From;
		Point To;

		Vector(Point from = Point(), Point to = Point());

		Vector(int32_t fromX, int32_t fromY, int32_t toX, int32_t toY);

		bool operator==(const Vector& other) const;
		bool operator!=(const Vector& other) const;
	};

	struct Size
	{
		static const Size Zero;
		static const Size Auto;

		int32_t Width;
		int32_t Height;

		Size(int32_t width = 0, int32_t height = 0);

		Size(Vector diagonal);

		bool operator==(const Size& other) const;
		bool operator!=(const Size& other) const;
	};

	struct Thickness
	{
		static const Thickness Zero;
		static const Thickness Single;

		int32_t Left;
		int32_t Top;
		int32_t Right;
		int32_t Bottom;

		Thickness(int32_t uniform = 0);

		Thickness(int32_t left, int32_t top, int32_t right, int32_t bottom);

		bool operator==(const Thickness& other) const;
		bool operator!=(const Thickness& other) const;

		bool IsUniform() const;

		int32_t Horizontal() const;

		int32_t Vertical() const;
	};

	struct Rect
	{
		int32_t X;
		int32_t Y;
		int32_t Width;
		int32_t Height;

		Rect(int32_t x = 0, int32_t y = 0, int32_t width = 0, int32_t height = 0);

		bool operator==(const Rect& other) const;
		bool operator!=(const Rect& other) const;

		bool IsEmpty() const;
		int32_t Right() const;
		int32_t Bottom() const;
		Size AsSize() const;
		
		bool Contains(Point point) const;
		bool Contains(Rect inner) const;
		bool Intersects(Rect other) const;

		static Rect Union(const Rect& a, const Rect& b);
		static Rect Enclose(const Rect& into, const Rect& rect);
		static Rect Clip(const Rect& into, const Rect& rect);
	};
}
