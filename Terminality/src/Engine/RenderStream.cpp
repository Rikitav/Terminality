
#include <cstdint>
#include <string>

#include <terminality/Engine/RenderContext.hpp>
#include <terminality/Engine/RenderStream.hpp>

using namespace terminality;

RenderStream::RenderStream(RenderContext& context, Point startPos)
    : context_(context), pos_(startPos), fg_(Color::WHITE), bg_(Color::BLACK), wrap_(false) { }

RenderStream& RenderStream::operator<<(const Point& point)
{
    pos_ = point;
    return *this;
}

RenderStream& RenderStream::operator<<(const RenderStreamColor& color)
{
    fg_ = color.Foreground.value_or(fg_);
    bg_ = color.Background.value_or(bg_);
    return *this;
}

RenderStream& RenderStream::operator<<(const std::wstring& text)
{
    if (text.empty())
        return *this;

    context_.RenderText(pos_, text, fg_, bg_, wrap_);

    Rect bounds = context_.ContextRect();
    if (wrap_)
    {
        pos_.X += static_cast<int32_t>(text.length());
        while (pos_.X >= bounds.Width && bounds.Width > 0)
        {
            pos_.X -= bounds.Width;
            pos_.Y += 1;
        }
    }
    else
    {
        pos_.X = std::min(pos_.X + static_cast<int32_t>(text.length()), bounds.Width);
    }

    return *this;
}

RenderStream& RenderStream::operator<<(const std::string& text)
{
    return *this << std::wstring(text.begin(), text.end());
}

RenderStream& RenderStream::operator<<(const wchar_t* text)
{
    if (text == nullptr)
        return *this;

    return *this << std::wstring(text);
}

RenderStream& RenderStream::operator<<(const char* text)
{
    if (text == nullptr)
        return *this;

    return *this << std::string(text);
}

RenderStream& RenderStream::operator<<(int32_t value)
{
    return *this << std::to_wstring(value);
}
RenderStream& RenderStream::operator<<(uint32_t value)
{
    return *this << std::to_wstring(value);
}
RenderStream& RenderStream::operator<<(float value)
{
    return *this << std::to_wstring(value);
}
RenderStream& RenderStream::operator<<(double value)
{
    return *this << std::to_wstring(value);
}

RenderStream& RenderStream::operator<<(RenderStream& (*manipulator)(RenderStream&))
{
    return manipulator(*this);
}

void RenderStream::NewLine()
{
    pos_.X = 0; pos_.Y += 1;
}

namespace terminality
{
    RenderStreamColor SetColor(Color fg, Color bg)
    {
        return RenderStreamColor{ fg, bg };
    }

    RenderStreamColor SetBack(Color bg)
    {
        return RenderStreamColor{ std::nullopt, bg };
    }

    RenderStreamColor SetFore(Color fg)
    {
        return RenderStreamColor{ fg, std::nullopt };
    }

    RenderStream& endl(RenderStream& stream)
    {
        stream.NewLine();
        return stream;
    }
}
