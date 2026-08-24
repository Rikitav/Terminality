
#include <cstdint>
#include <mutex>
#include <algorithm>

#include <terminality/Core/Geometry.hpp>
#include <terminality/Engine/RenderContext.hpp>

using namespace terminality;

RenderContext::RenderContext(RenderBuffer& buffer, Rect targetRect)
    : buffer_(buffer), rect_(targetRect), clipRect_(targetRect) {}

RenderContext RenderContext::CreateInner(Rect targetRect)
{
    RenderContext inner(buffer_, targetRect);
    inner.clipRect_ = Rect::Clip(clipRect_, targetRect);
    return inner;
}

Rect RenderContext::ContextRect() const
{
    return rect_;
}

void RenderContext::SetCell(int32_t x, int32_t y, const CellInfo& cell)
{
    if (x < 0 || y < 0 || x >= rect_.Width || y >= rect_.Height)
        return;

    int32_t absX = rect_.X + x;
    int32_t absY = rect_.Y + y;

    if (!clipRect_.Contains(Point(absX, absY)))
        return;

    buffer_.SetCell(static_cast<uint32_t>(absX), static_cast<uint32_t>(absY), cell);
}

void RenderContext::SetCell(int32_t x, int32_t y, const wchar_t puts, Color fg, Color bg)
{
    SetCell(x, y, CellInfo{ puts, fg, bg });
}

CellInfo RenderContext::GetCell(int32_t x, int32_t y) const
{
    if (x < 0 || y < 0 || x >= rect_.Width || y >= rect_.Height)
        return CellInfo();

    int32_t absX = rect_.X + x;
    int32_t absY = rect_.Y + y;

    if (!clipRect_.Contains(Point(absX, absY)))
        return CellInfo();

    return buffer_.GetCell(static_cast<uint32_t>(absX), static_cast<uint32_t>(absY));
}

void RenderContext::RenderRaw(const Point& point, const std::string& rawData)
{
    std::lock_guard<std::recursive_mutex> guard(buffer_.renderMutex);

    for (int32_t i = 0; i < static_cast<int32_t>(rawData.size()); ++i)
    {
        SetCell(point.X + i, point.Y, CellInfo{ static_cast<wchar_t>(rawData[i]) });
    }
}

RenderStream RenderContext::BeginText(Point startPos)
{
    return RenderStream(*this, startPos);
}

void RenderContext::RenderText(const Point& point, const std::wstring& text, Color fg, Color bg, bool wrap)
{
    if (text.empty())
        return;

    std::lock_guard<std::recursive_mutex> guard(buffer_.renderMutex);

    int32_t x = point.X;
    int32_t y = point.Y;

    for (wchar_t ch : text)
    {
        if (x >= rect_.Width)
        {
            if (!wrap)
                break;

            x = 0;
            y++;
        }

        if (y < 0 || y >= rect_.Height)
            break;

        if (x >= 0)
            SetCell(x, y, CellInfo{ ch, fg, bg });

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
