export module terminality:TextHelper;

import std;
import std.compat;
import :Layout;

export namespace terminality
{
    struct LineBounds
    {
        size_t Start;
        size_t End;
        size_t NextStart;
    };

	struct LineInfo
	{
		std::wstring Text;
		size_t StartIndex;
	};

	class TextHelper
	{
    public:
        static std::vector<LineBounds> CalculateLineBounds(const std::wstring& text, int32_t availableWidth, TextWrapping wrapping);
        static std::vector<LineInfo> GetLines(const std::wstring& text, int32_t availableWidth, TextWrapping wrapping);
        static std::vector<int32_t> MeasureLines(const std::wstring& text, int32_t availableWidth, TextWrapping wrapping);
	};
}
