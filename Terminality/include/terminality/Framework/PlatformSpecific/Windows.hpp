#pragma once

#include <string>

#ifdef _WIN32
namespace terminality
{
    void AlertAsync(const std::wstring& text, const std::wstring& title);
}

#endif // _WIN32
