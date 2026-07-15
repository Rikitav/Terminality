#ifdef _WIN32

#include <string>
#include <thread>
#include <memory>

#include <terminality/Terminality.hpp>
#include <Windows.h>

using namespace terminality;

void terminality::AlertAsync(const std::wstring& text, const std::wstring& title)
{
    std::thread([text, title]() { MessageBoxW(nullptr, text.c_str(), title.size() == 0 ? nullptr : title.c_str(), MB_OK | MB_ICONINFORMATION); }).detach();
}

#endif // _WIN32
