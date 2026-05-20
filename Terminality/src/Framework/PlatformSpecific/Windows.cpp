module;

#ifdef _WIN32
#include <Windows.h>
#include <string>
#include <thread>
#include <memory>
#endif

module terminality;

using namespace terminality;

#ifdef _WIN32

void terminality::AlertAsync(const std::wstring& text, const std::wstring& title)
{
    std::thread([text, title]() { MessageBoxW(nullptr, text.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION); }).detach();
}

#endif // _WIN32
