export module terminality:RenderContext;

import std;
import std.compat;
import :Geometry;
import :RenderBuffer;
import :RenderStream;

export namespace terminality
{
	typedef wchar_t (*RectangleStyle)(const Point& point, const Size& size);
	typedef wchar_t (*VectorStyle)(const Point& point, const Vector& vector);

	class RenderContext
	{
		RenderBuffer& buffer_;
		Rect rect_;

	public:
		RenderContext(RenderBuffer& buffer, Rect targetRect);
		RenderContext CreateInner(Rect targetRect);

		Rect ContextRect();

		void SetCell(uint32_t x, uint32_t y, const CellInfo& cell);
		void SetCell(uint32_t x, uint32_t y, const wchar_t puts, Color fg = Color::WHITE, Color bg = Color::BLACK);
		const CellInfo& GetCell(uint32_t x, uint32_t y) const;

		void RenderRaw(const Point& point, const std::string& rawData);
		RenderStream BeginText(Point startPos = Point(0, 0));

		void RenderText(const Point& point, const std::string& text, Color fg = Color::WHITE, Color bg = Color::BLACK, bool wrap = false);
		void RenderText(const Point& point, const std::wstring& text, Color fg = Color::WHITE, Color bg = Color::BLACK, bool wrap = false);
		void RenderText(const Point& point, const char* text, Color fg = Color::WHITE, Color bg = Color::BLACK, bool wrap = false);
		void RenderText(const Point& point, const wchar_t* text, Color fg = Color::WHITE, Color bg = Color::BLACK, bool wrap = false);

		void RenderRectangle(const Point& point, const Size& size);
		void RenderRectangle(const Point& point, const Size& size, RectangleStyle style);
		void RenderRectangle(const Point& point, const int32_t width, const int32_t height);
		void RenderRectangle(const Point& point, const int32_t width, const int32_t height, RectangleStyle style);

		void RenderLine(const Point& point, const int32_t length, short direction = 0);
		void RenderLine(const Point& point, const int32_t length, VectorStyle style);
		void RenderLine(const Point& fromPoint, const Point& toPoint);
		void RenderLine(const Point& fromPoint, const Point& toPoint, VectorStyle style);
		void RenderLine(const Vector& vector);
		void RenderLine(const Vector& vector, VectorStyle style);
	};
}
