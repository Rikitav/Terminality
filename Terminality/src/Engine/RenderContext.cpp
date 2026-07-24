
#include <cstdint>
#include <mutex>
#include <algorithm>

#include <terminality/Core/Geometry.hpp>
#include <terminality/Engine/RenderContext.hpp>

using namespace terminality;

RenderContext::RenderContext(RenderBuffer& buffer, Rect targetRect)
    : buffer_(buffer), rect_(targetRect) {}

RenderContext RenderContext::CreateInner(Rect targetRect)
{
	int32_t x = targetRect.X;
	int32_t y = targetRect.Y;
	
	int32_t right = std::clamp<int32_t>(x + targetRect.Width, rect_.X, rect_.Right());
    int32_t bottom = std::clamp<int32_t>(y + targetRect.Height, rect_.Y, rect_.Bottom());

	int32_t width = std::max(0, right - x);
	int32_t height = std::max(0, bottom - y);

	return RenderContext(buffer_, Rect(x, y, width, height));
}

Rect RenderContext::ContextRect() const
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
}

CellInfo RenderContext::GetCell(uint32_t x, uint32_t y) const
{
    if (x >= static_cast<uint32_t>(rect_.Width))
        return CellInfo();

    if (y >= static_cast<uint32_t>(rect_.Height))
        return CellInfo();

    int32_t absX = rect_.X + x;
    int32_t absY = rect_.Y + y;

    return buffer_.GetCell(absX, absY);
}

void RenderContext::RenderRaw(const Point& point, const std::string& rawData)
{
    std::lock_guard<std::recursive_mutex> guard(buffer_.renderMutex);

    if (point.Y < 0 || point.Y >= rect_.Height)
        return;

    for (int32_t i = 0; i < static_cast<int32_t>(rawData.size()); ++i)
    {
        int32_t localX = point.X + i;
        if (localX < 0 || localX >= rect_.Width)
            continue;

        buffer_.SetCell(rect_.X + localX, rect_.Y + point.Y, CellInfo{ static_cast<wchar_t>(rawData[i]) });
    }
}

RenderStream RenderContext::BeginText(Point startPos)
{
    return RenderStream(*this, startPos);
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
}

void RenderContext::RenderText(const Point& point, const std::string& text, Color fg, Color bg, bool wrap)
{
    std::wstring wtext(text.begin(), text.end());
    RenderText(point, wtext, fg, bg, wrap);
}

void RenderContext::RenderText(const Point& point, const char* text, Color fg, Color bg, bool wrap)
{
    if (text == nullptr)
        return;

    RenderText(point, std::string(text), fg, bg, wrap);
}

void RenderContext::RenderText(const Point& point, const wchar_t* text, Color fg, Color bg, bool wrap)
{
    if (text == nullptr)
        return;

    RenderText(point, std::wstring(text), fg, bg, wrap);
}

void RenderContext::RenderRectangle(const Point& point, const Size& size, RectangleStyle style)
{
    std::lock_guard<std::recursive_mutex> guard(buffer_.renderMutex);

    for (int32_t y = 0; y < size.Height; ++y)
    {
        for (int32_t x = 0; x < size.Width; ++x)
        {
            wchar_t symbol = style(Point(x, y), size);
            if (symbol != L'\0')
                SetCell(point.X + x, point.Y + y, CellInfo(symbol));
        }
    }
}

void RenderContext::RenderRectangle(const Point& point, const Size& size, Color fg, Color bg, RectangleStyle style)
{
    std::lock_guard<std::recursive_mutex> guard(buffer_.renderMutex);

    for (int32_t y = 0; y < size.Height; ++y)
    {
        for (int32_t x = 0; x < size.Width; ++x)
        {
            wchar_t symbol = style(Point(x, y), size);
            if (symbol != L'\0')
                SetCell(point.X + x, point.Y + y, CellInfo(symbol, fg, bg));
        }
    }
}

static wchar_t DefaultRectangleStyle(const Point& point, const Size& size)
{
    return L'#';
}

void RenderContext::RenderRectangle(const Point& point, const Size& size)
{
    RenderRectangle(point, size, DefaultRectangleStyle);
}

void RenderContext::RenderRectangle(const Point& point, const Size& size, Color fg, Color bg)
{
    RenderRectangle(point, size, fg, bg, DefaultRectangleStyle);
}

void RenderContext::RenderRectangle(const Point& point, const int32_t width, const int32_t height)
{
    RenderRectangle(point, Size(width, height));
}

void RenderContext::RenderRectangle(const Point& point, const int32_t width, const int32_t height, Color fg, Color bg)
{
    RenderRectangle(point, Size(width, height), fg, bg);
}

void RenderContext::RenderRectangle(const Point& point, const int32_t width, const int32_t height, RectangleStyle style)
{
    RenderRectangle(point, Size(width, height), style);
}

void RenderContext::RenderRectangle(const Point& point, const int32_t width, const int32_t height, Color fg, Color bg, RectangleStyle style)
{
    RenderRectangle(point, Size(width, height), fg, bg, style);
}

void RenderContext::RenderLine(const Vector& vector)
{
    RenderLine(vector.From, vector.To);
}

void RenderContext::RenderLine(const Vector& vector, Color fg, Color bg)
{
    RenderLine(vector.From, vector.To, fg, bg);
}

void RenderContext::RenderLine(const Vector& vector, VectorStyle style)
{
    RenderLine(vector.From, vector.To, style);
}

void RenderContext::RenderLine(const Vector& vector, VectorStyle style, Color fg, Color bg)
{
    RenderLine(vector.From, vector.To, style, fg, bg);
}

void RenderContext::RenderLine(const Point& fromPoint, const Point& toPoint)
{
    RenderLine(fromPoint, toPoint, Color::WHITE, Color::BLACK);
}

void RenderContext::RenderLine(const Point& fromPoint, const Point& toPoint, Color fg, Color bg)
{
    RenderLine(fromPoint, toPoint,
        [](const Point& current, const Vector& v)
        {
            int32_t dx = std::abs(v.To.X - v.From.X);
            int32_t dy = std::abs(v.To.Y - v.From.Y);
            return (dx > dy) ? L'\u2500' : L'\u2502';
        }, fg, bg);
}

void RenderContext::RenderLine(const Point& fromPoint, const Point& toPoint, VectorStyle style)
{
    RenderLine(fromPoint, toPoint, style, Color::WHITE, Color::BLACK);
}

void RenderContext::RenderLine(const Point& fromPoint, const Point& toPoint, VectorStyle style, Color fg, Color bg)
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
            SetCell(x1, y1, symbol, fg, bg);
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
    RenderLine(point, length, Color::WHITE, Color::BLACK, direction);
}

void RenderContext::RenderLine(const Point& point, const int32_t length, Color fg, Color bg, short direction)
{
    if (length <= 0)
        return;

    Point to = point;
    if (direction == 0)
        to.X += length - 1;
    else
        to.Y += length - 1;

    RenderLine(point, to, fg, bg);
}

void RenderContext::RenderLine(const Point& point, const int32_t length, VectorStyle style, short direction)
{
    RenderLine(point, length, style, Color::WHITE, Color::BLACK, direction);
}

void RenderContext::RenderLine(const Point& point, const int32_t length, VectorStyle style, Color fg, Color bg, short direction)
{
    if (length <= 0)
        return;

    Point to = point;
    if (direction == 0)
        to.X += length - 1;
    else
        to.Y += length - 1;

    RenderLine(point, to, style, fg, bg);
}
