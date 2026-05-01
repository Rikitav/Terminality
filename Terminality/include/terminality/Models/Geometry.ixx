export module terminality:Geometry;

import std;
import std.compat;

export namespace terminality
{
	enum class Color
	{
		BLACK = 0,
		DARK_BLUE = 1,
		DARK_GREEN = 2,
		DARK_CYAN = 3,
		DARK_RED = 4,
		DARK_MAGENTA = 5,
		DARK_YELLOW = 6,
		LIGHT_GRAY = 7,
		DARK_GRAY = 8,
		BLUE = 9,
		GREEN = 10,
		CYAN = 11,
		RED = 12,
		MAGENTA = 13,
		YELLOW = 14,
		WHITE = 15
	};

	struct Point
	{
		int32_t X;
		int32_t Y;

		Point(int32_t x = 0, int32_t y = 0)
			: X(x), Y(y) {
		}

		bool operator==(const Point& other) const;
		bool operator!=(const Point& other) const;
	};

	struct Vector
	{
		Point From;
		Point To;

		Vector(Point from = Point(), Point to = Point())
			: From(from), To(to) {
		}

		Vector(int32_t fromX, int32_t fromY, int32_t toX, int32_t toY)
			: From(fromX, fromY), To(toX, toY) {
		}

		bool operator==(const Vector& other) const;
		bool operator!=(const Vector& other) const;
	};

	struct Size
	{
		static const Size Zero;
		static const Size Auto;

		int32_t Width;
		int32_t Height;

		Size(int32_t width = 0, int32_t height = 0)
			: Width(width), Height(height) {
		}

		Size(Vector diagonal)
			: Width(std::max(0, diagonal.To.X - diagonal.From.X)), Height(std::max(0, diagonal.To.Y - diagonal.From.Y)) {
		}

		bool operator==(const Size& other) const;
		bool operator!=(const Size& other) const;
	};

	struct Thickness
	{
		int32_t Left;
		int32_t Top;
		int32_t Right;
		int32_t Bottom;

		Thickness(int32_t uniform = 0)
			: Left(uniform), Top(uniform), Right(uniform), Bottom(uniform) {
		}

		Thickness(int32_t left, int32_t top, int32_t right, int32_t bottom)
			: Left(left), Top(top), Right(right), Bottom(bottom) {
		}

		bool operator==(const Thickness& other) const;
		bool operator!=(const Thickness& other) const;

		bool IsUniform() const;

		int32_t Horizontal() const {
			return Left + Right;
		}
		int32_t Vertical() const {
			return Top + Bottom;
		}
	};

	struct Rect
	{
		int32_t X;
		int32_t Y;
		int32_t Width;
		int32_t Height;

		Rect(int32_t x = 0, int32_t y = 0, int32_t width = 0, int32_t height = 0)
			: X(x), Y(y), Width(width), Height(height) {
		}

		bool operator==(const Rect& other) const;
		bool operator!=(const Rect& other) const;

		bool IsEmpty() const;
		int32_t Right() const;
		int32_t Bottom() const;
		Size Size() const;

		static Rect Union(const Rect& a, const Rect& b);
		static Rect Enclose(const Rect& into, const Rect& rect);
		static Rect Clip(const Rect& into, const Rect& rect);
	};
}
