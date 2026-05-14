module terminality;

import std;
import std.compat;

using namespace terminality;

RenderStream& RenderStream::operator<<(const std::wstring& text)
{
    if (text.empty())
        return *this;

    context_.RenderText(pos_, text, fg_, bg_, wrap_);
    pos_.X += static_cast<int32_t>(text.length());
    return *this;
}

RenderStream& RenderStream::operator<<(const std::string& text)
{
    return *this << std::wstring(text.begin(), text.end());
}

RenderStream& RenderStream::operator<<(const wchar_t* text)
{
    return *this << std::wstring(text);
}

RenderStream& RenderStream::operator<<(const char* text)
{
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
