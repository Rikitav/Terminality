module terminality;

import std;
import std.compat;

using namespace terminality;

std::vector<LineBounds> TextHelper::CalculateLineBounds(const std::wstring& text, int32_t availableWidth, TextWrapping wrapping)
{
    std::vector<LineBounds> bounds;
    if (text.empty())
    {
        bounds.push_back({ 0, 0, 0 });
        return bounds;
    }

    const size_t len = text.length();
    size_t start = 0;

    while (start < len)
    {
        size_t end = start;
        int32_t width = 0;
        size_t lastSpace = std::wstring::npos;

        if (wrapping == TextWrapping::NoWrap || availableWidth <= 0)
        {
            while (end < len && text[end] != L'\n')
                end++;

            bounds.push_back({ start, end, end < len ? end + 1 : len });
            start = end < len ? end + 1 : len;
            continue;
        }

        while (end < len && text[end] != L'\n' && width < availableWidth)
        {
            if (text[end] == L' ')
                lastSpace = end;

            end++;
            width++;
        }

        if (end == len || text[end] == L'\n')
        {
            bounds.push_back({ start, end, end < len ? end + 1 : len });
            start = end < len ? end + 1 : len;
            continue;
        }

        if (wrapping == TextWrapping::WrapWholeWords)
        {
            if (end < len && (text[end] == L' ' || text[end] == L'\n'))
            {
                bounds.push_back({ start, end, text[end] == L'\n' ? end + 1 : end + 1 });
                start = end + 1;
                continue;
            }

            if (lastSpace != std::wstring::npos && lastSpace >= start)
            {
                bounds.push_back({ start, lastSpace, lastSpace + 1 });
                start = lastSpace + 1;
                continue;
            }
        }

        bounds.push_back({ start, end, end });
        start = end;
    }

    if (!text.empty() && text.back() == L'\n')
        bounds.push_back({ len, len, len });

    return bounds;
}

std::vector<LineInfo> TextHelper::GetLines(const std::wstring& text, int32_t availableWidth, TextWrapping wrapping)
{
    std::vector<LineInfo> lines;
    auto bounds = CalculateLineBounds(text, availableWidth, wrapping);

    lines.reserve(bounds.size());
    for (const auto& b : bounds)
    {
        lines.push_back({ text.substr(b.Start, b.End - b.Start), b.Start });
    }

    return lines;
}

std::vector<int32_t> TextHelper::MeasureLines(const std::wstring& text, int32_t availableWidth, TextWrapping wrapping)
{
    std::vector<int32_t> lineLengths;
    auto bounds = CalculateLineBounds(text, availableWidth, wrapping);

    lineLengths.reserve(bounds.size());
    for (const auto& b : bounds)
    {
        lineLengths.push_back(static_cast<int32_t>(b.End - b.Start));
    }

    return lineLengths;
}
