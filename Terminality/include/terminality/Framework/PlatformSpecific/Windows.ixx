module;

#ifdef _WIN32
#include <string>
#endif

export module terminality:Windows;

#ifdef _WIN32
export namespace terminality
{
    void AlertAsync(const std::wstring& text, const std::wstring& title);
}

#endif // _WIN32
