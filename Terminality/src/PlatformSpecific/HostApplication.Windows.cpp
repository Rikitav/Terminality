module;

#ifdef _WIN32
#include <clocale>
#include <Windows.h>
#endif

module terminality;

import std;
import std.compat;

using namespace terminality;

#ifdef _WIN32
HostApplication* HostApplication::Current()
{
    static HostApplication app;
	app.focusManager_ = &FocusManager::Current();
	app.visualTree_ = new VisualTree();
	return &app;
}

void HostApplication::EnterTerminal()
{
    std::setlocale(LC_ALL, ".UTF-8");
    try
    {
        std::locale::global(std::locale(".UTF-8"));
        std::cout.imbue(std::locale());
        std::cerr.imbue(std::locale());
    }
    catch (...)
    {
        // Locale not available; keep process defaults.
    }
    
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    DWORD dwMode = 0;
    static HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);

    GetConsoleMode(hInput, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hInput, dwMode);

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD inMode = 0;
    if (GetConsoleMode(hIn, &inMode))
        SetConsoleMode(hIn, inMode | ENABLE_WINDOW_INPUT);

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hInput, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hInput, &cursorInfo);

	std::wcout << L"\x1b[2J\x1b[?25l";
}

void HostApplication::ExitTerminal()
{
	std::wcout << L"\x1b[0m\x1b[?25h";
	std::wcout.flush();
}

#endif // _WIN32
