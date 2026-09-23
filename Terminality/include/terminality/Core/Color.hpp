#pragma once

#include <cstdint>

namespace terminality
{
	/// 24-bit RGB color. The named constants mirror the classic 16-color console
	/// palette (standard VGA values), so existing code keeps its look while any
	/// custom RGB value can be used directly.
	struct Color
	{
		uint8_t R = 0;
		uint8_t G = 0;
		uint8_t B = 0;

		/// When set, the color is treated as "unspecified": RenderBuffer keeps the
		/// underlying cell color when compositing and emits no SGR sequence.
		bool Transparent = false;

		constexpr Color() = default;
		constexpr Color(uint8_t r, uint8_t g, uint8_t b) : R(r), G(g), B(b) { }

		bool operator==(const Color& other) const = default;
		bool operator!=(const Color& other) const = default;

		/// Linear interpolation between two opaque colors; t = 0 gives `from`,
		/// t = 1 gives `to`. Transparent colors are returned as-is.
		static constexpr Color Lerp(Color from, Color to, float t)
		{
			if (from.Transparent || to.Transparent || t <= 0.0f)
				return from;
			if (t >= 1.0f)
				return to;

			return Color(
				static_cast<uint8_t>(from.R + (to.R - from.R) * t),
				static_cast<uint8_t>(from.G + (to.G - from.G) * t),
				static_cast<uint8_t>(from.B + (to.B - from.B) * t));
		}

		/// Scales the color toward black; amount 0 keeps it, 1 makes it black.
		static constexpr Color Darken(Color color, float amount)
		{
			if (color.Transparent || amount <= 0.0f)
				return color;
			if (amount >= 1.0f)
				return Color(0, 0, 0);

			const float scale = 1.0f - amount;
			return Color(
				static_cast<uint8_t>(color.R * scale),
				static_cast<uint8_t>(color.G * scale),
				static_cast<uint8_t>(color.B * scale));
		}

		static const Color TRANSPARENT;

		static const Color BLACK;
		static const Color DARK_BLUE;
		static const Color DARK_GREEN;
		static const Color DARK_CYAN;
		static const Color DARK_RED;
		static const Color DARK_MAGENTA;
		static const Color DARK_YELLOW;
		static const Color LIGHT_GRAY;
		static const Color DARK_GRAY;
		static const Color BLUE;
		static const Color GREEN;
		static const Color CYAN;
		static const Color RED;
		static const Color MAGENTA;
		static const Color YELLOW;
		static const Color WHITE;

	private:
		constexpr Color AsTransparent() const
		{
			Color copy = *this;
			copy.Transparent = true;
			return copy;
		}
	};

	// Inline definitions live outside the struct so Color is a complete type here.
	inline const Color Color::TRANSPARENT = Color(0, 0, 0).AsTransparent();

	inline const Color Color::BLACK        = Color(0, 0, 0);
	inline const Color Color::DARK_BLUE    = Color(0, 0, 128);
	inline const Color Color::DARK_GREEN   = Color(0, 128, 0);
	inline const Color Color::DARK_CYAN    = Color(0, 128, 128);
	inline const Color Color::DARK_RED     = Color(128, 0, 0);
	inline const Color Color::DARK_MAGENTA = Color(128, 0, 128);
	inline const Color Color::DARK_YELLOW  = Color(128, 128, 0);
	inline const Color Color::LIGHT_GRAY   = Color(192, 192, 192);
	inline const Color Color::DARK_GRAY    = Color(128, 128, 128);
	inline const Color Color::BLUE         = Color(0, 0, 255);
	inline const Color Color::GREEN        = Color(0, 255, 0);
	inline const Color Color::CYAN         = Color(0, 255, 255);
	inline const Color Color::RED          = Color(255, 0, 0);
	inline const Color Color::MAGENTA      = Color(255, 0, 255);
	inline const Color Color::YELLOW       = Color(255, 255, 0);
	inline const Color Color::WHITE        = Color(255, 255, 255);
}
