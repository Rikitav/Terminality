export module terminality:RenderContext;

import std;
import std.compat;
import :Geometry;
import :RenderBuffer;

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

    RenderContext::RenderContext(RenderBuffer& buffer, Rect targetRect)
        : buffer_(buffer), rect_(targetRect) { }

    RenderContext RenderContext::CreateInner(Rect targetRect)
    {
        return RenderContext(buffer_, Rect::Enclose(rect_, targetRect));
    }

    Rect RenderContext::ContextRect()
    {
        return rect_;
    }

    void RenderContext::SetCell(uint32_t x, uint32_t y, const CellInfo& cell)
    {
        if (x >= static_cast<uint32_t>(rect_.Width))
            return;

        if (y >= static_cast<uint32_t>(rect_.Height))
            return;

        int32_t absX = rect_.X + x;
        int32_t absY = rect_.Y + y;

        buffer_.SetCell(absX, absY, cell);
        buffer_.MarkDirty(Rect(absX, absY, 1, 1));
    }

    void RenderContext::SetCell(uint32_t x, uint32_t y, const wchar_t puts, Color fg, Color bg)
    {
        if (x >= static_cast<uint32_t>(rect_.Width))
            return;

        if (y >= static_cast<uint32_t>(rect_.Height))
            return;

        int32_t absX = rect_.X + x;
        int32_t absY = rect_.Y + y;

        buffer_.SetCell(absX, absY, CellInfo{ puts, fg, bg });
        buffer_.MarkDirty(Rect(absX, absY, 1, 1));
    }

    const CellInfo& RenderContext::GetCell(uint32_t x, uint32_t y) const
    {
        static CellInfo empty;
        if (x >= static_cast<uint32_t>(rect_.Width))
            return empty;

        if (y >= static_cast<uint32_t>(rect_.Height))
            return empty;

        int32_t absX = rect_.X + x;
        int32_t absY = rect_.Y + y;

        return buffer_.GetCell(absX, absY);
    }

    void RenderContext::RenderRaw(const Point& point, const std::string& rawData)
    {
        std::lock_guard<std::recursive_mutex> guard(buffer_.renderMutex);

        uint32_t absX = rect_.X + point.X;
        uint32_t absY = rect_.Y + point.Y;

        for (int32_t i = 0; i < static_cast<int32_t>(rawData.size()); ++i)
        {
            if (point.X + i < rect_.Width)
            {
                buffer_.SetCell(absX + i, absY, CellInfo{ (wchar_t)rawData[i] });
            }
        }
    }

    void RenderContext::RenderText(const Point& point, const std::wstring& text, Color fg, Color bg, bool wrap)
    {
        if (text.empty() || point.Y >= rect_.Height || point.X >= rect_.Width) return;

        std::lock_guard<std::recursive_mutex> guard(buffer_.renderMutex);

        uint32_t x = point.X;
        uint32_t y = point.Y;

        for (wchar_t ch : text)
        {
            if (x >= static_cast<uint32_t>(rect_.Width))
            {
                if (!wrap)
                    break;

                x = 0;
                y++;
            }

            if (y >= static_cast<uint32_t>(rect_.Height))
                break;

            buffer_.SetCell(rect_.X + x, rect_.Y + y, CellInfo{ ch, fg, bg });
            x++;
        }

        buffer_.MarkDirty(Rect(rect_.X + point.X, rect_.Y + point.Y, (int32_t)text.size(), 1));
    }

    void RenderContext::RenderText(const Point& point, const std::string& text, Color fg, Color bg, bool wrap)
    {
        std::wstring wtext(text.begin(), text.end());
        RenderText(point, wtext, fg, bg, wrap);
    }

    void RenderContext::RenderText(const Point& point, const char* text, Color fg, Color bg, bool wrap)
    {
        RenderText(point, std::string(text), fg, bg, wrap);
    }

    void RenderContext::RenderText(const Point& point, const wchar_t* text, Color fg, Color bg, bool wrap)
    {
        RenderText(point, std::wstring(text), fg, bg, wrap);
    }

    void RenderContext::RenderRectangle(const Point& point, const Size& size, RectangleStyle style)
    {
        std::lock_guard<std::recursive_mutex> guard(buffer_.renderMutex);

        for (int32_t y = 0; y < size.Height; ++y)
        {
            for (int32_t x = 0; x < size.Width; ++x)
            {
                wchar_t symbol = style({ x, y }, size);
                if (symbol != L'\0')
                    SetCell(point.X + x, point.Y + y, CellInfo{ symbol });
            }
        }
    }

    void RenderContext::RenderRectangle(const Point& point, const Size& size)
    {
        RenderRectangle(point, size, [](const Point&, const Size&) { return L'#'; });
    }

    void RenderContext::RenderRectangle(const Point& point, const int32_t width, const int32_t height)
    {
        RenderRectangle(point, Size(width, height));
    }

    void RenderContext::RenderRectangle(const Point& point, const int32_t width, const int32_t height, RectangleStyle style)
    {
        RenderRectangle(point, Size(width, height), style);
    }

    void RenderContext::RenderLine(const Vector& vector)
    {
        RenderLine(vector.From, vector.To);
    }

    void RenderContext::RenderLine(const Vector& vector, VectorStyle style)
    {
        RenderLine(vector.From, vector.To, style);
    }

    void RenderContext::RenderLine(const Point& fromPoint, const Point& toPoint)
    {
        RenderLine(fromPoint, toPoint, [](const Point& current, const Vector& v)
            {
                int32_t dx = std::abs(v.To.X - v.From.X);
                int32_t dy = std::abs(v.To.Y - v.From.Y);
                return (dx > dy) ? L'─' : L'│';
            });
    }

    void RenderContext::RenderLine(const Point& fromPoint, const Point& toPoint, VectorStyle style)
    {
        std::lock_guard<std::recursive_mutex> guard(buffer_.renderMutex);

        int32_t x1 = fromPoint.X;
        int32_t y1 = fromPoint.Y;
        int32_t x2 = toPoint.X;
        int32_t y2 = toPoint.Y;

        int32_t dx = std::abs(x2 - x1);
        int32_t dy = std::abs(y2 - y1);
        int32_t sx = (x1 < x2) ? 1 : -1;
        int32_t sy = (y1 < y2) ? 1 : -1;
        int32_t err = dx - dy;

        Vector v(fromPoint, toPoint);

        while (true)
        {
            wchar_t symbol = style(Point(x1, y1), v);
            if (symbol != L'\0')
            {
                SetCell(x1, y1, CellInfo{ symbol });
            }

            if (x1 == x2 && y1 == y2)
                break;

            int32_t e2 = 2 * err;
            if (e2 > -dy)
            {
                err -= dy;
                x1 += sx;
            }

            if (e2 < dx)
            {
                err += dx;
                y1 += sy;
            }
        }
    }

    void RenderContext::RenderLine(const Point& point, const int32_t length, short direction)
    {
        Point to = point;
        if (direction == 0)
        {
            to.X += (length > 0 ? length - 1 : 0);
        }
        else
        {
            to.Y += (length > 0 ? length - 1 : 0);
        }

        RenderLine(point, to);
    }

    void RenderContext::RenderLine(const Point& point, const int32_t length, VectorStyle style)
    {
        Point to(point.X + (length > 0 ? length - 1 : 0), point.Y);
        RenderLine(point, to, style);
    }
}
