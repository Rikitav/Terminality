export module terminality:TextHelper;

import std;
import std.compat;
import :Layout;

export namespace terminality
{
    struct LineBounds
    {
        std::size_t Start;
        std::size_t End;
        std::size_t NextStart;
    };

	struct LineInfo
	{
		std::wstring Text;
		std::size_t StartIndex;
	};

	class TextHelper
	{
    public:
        static std::vector<LineBounds> CalculateLineBounds(const std::wstring& text, int32_t availableWidth, TextWrap wrapping);
        static std::vector<LineInfo> GetLines(const std::wstring& text, int32_t availableWidth, TextWrap wrapping);
        static std::vector<int32_t> MeasureLines(const std::wstring& text, int32_t availableWidth, TextWrap wrapping);
	};
}
