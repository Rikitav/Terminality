#pragma once

#include <cstdint>
#include <string>
#include <optional>

#include <terminality/Core/Color.hpp>
#include <terminality/Core/Layout.hpp>
#include <terminality/Core/Geometry.hpp>

namespace terminality
{
    class RenderContext;

    struct RenderStreamColor
    {
        std::optional<Color> Foreground;
        std::optional<Color> Background;
    };

    class RenderStream
    {
        RenderContext& context_;
        Point pos_;
        Color fg_;
        Color bg_;
        bool wrap_;

    public:
        RenderStream(RenderContext& context, Point startPos = Point(0, 0));

        RenderStream& operator<<(const Point& point);

        RenderStream& operator<<(const RenderStreamColor& color);

        RenderStream& operator<<(const std::wstring& text);
        RenderStream& operator<<(const std::string& text);
        RenderStream& operator<<(const wchar_t* text);
        RenderStream& operator<<(const char* text);
        RenderStream& operator<<(int32_t value);
        RenderStream& operator<<(uint32_t value);
        RenderStream& operator<<(float value);
        RenderStream& operator<<(double value);
        RenderStream& operator<<(RenderStream& (*manipulator)(RenderStream&));

        void NewLine();
    };

    RenderStreamColor SetColor(Color fg, Color bg = Color::BLACK);

    RenderStreamColor SetBack(Color bg);

    RenderStreamColor SetFore(Color fg);

    RenderStream& endl(RenderStream& stream);
}
