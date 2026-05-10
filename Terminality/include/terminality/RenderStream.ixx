export module terminality:RenderStream;

import std;
import std.compat;
import :Layout;
import :Geometry;

export namespace terminality
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
        RenderStream(RenderContext& context, Point startPos = Point(0, 0))
            : context_(context), pos_(startPos), fg_(Color::WHITE), bg_(Color::BLACK), wrap_(false) {}

        RenderStream& operator<<(const Point& point)
        {
            pos_ = point;
            return *this;
        }

        RenderStream& operator<<(const RenderStreamColor& color)
        {
            fg_ = color.Foreground.value_or(fg_);
            bg_ = color.Background.value_or(bg_);
            return *this;
        }

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

    inline RenderStreamColor SetColor(Color fg, Color bg = Color::BLACK)
    {
        return RenderStreamColor{ fg, bg };
    }

    inline RenderStreamColor SetBack(Color bg)
    {
        return RenderStreamColor{ std::nullopt, bg };
    }

    inline RenderStreamColor SetFore(Color fg)
    {
        return RenderStreamColor{ fg, std::nullopt };
    }

    inline RenderStream& endl(RenderStream& stream)
    {
        stream.NewLine();
        return stream;
    }
}
