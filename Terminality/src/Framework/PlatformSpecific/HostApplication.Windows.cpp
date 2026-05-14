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
void HostApplication::EnterTerminal()
{
    std::setlocale(LC_ALL, ".UTF-8");
    try
    {
        std::locale::global(std::locale(".UTF-8"));
        std::wcout.imbue(std::locale());
        std::wcerr.imbue(std::locale());
    }
    catch (...)
    {
        // Locale not available; keep process defaults.
    }
    
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD outMode = 0;
    if (GetConsoleMode(hOutput, &outMode))
    {
        outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOutput, outMode);
    }

    CONSOLE_CURSOR_INFO cursorInfo;
    if (GetConsoleCursorInfo(hOutput, &cursorInfo))
    {
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hOutput, &cursorInfo);
    }

    DWORD inMode = 0;
    if (GetConsoleMode(hInput, &inMode))
        SetConsoleMode(hInput, inMode | ENABLE_WINDOW_INPUT);

    std::ios_base::sync_with_stdio(false);
    std::wcout.tie(nullptr);
    std::wcout << L"\x1b[?1049h\x1b[0m\x1b[40m\x1b[?7l\x1b[2J\x1b[?25l";
}

void HostApplication::ExitTerminal()
{
    std::wcout << L"\x1b[?1049l\x1b[?7h\x1b[?25h";
	std::wcout.flush();
}

Size HostBackend::QueryViewportSize()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns, rows;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    return Size(columns, rows);
}

InputEvent HostBackend::PollInput(std::chrono::milliseconds timeout)
{
    static HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD waitResult = WaitForSingleObject(hInput, static_cast<DWORD>(timeout.count()));

    if (waitResult != WAIT_OBJECT_0)
        return InputEvent(InputModifier::None, InputKey::None, false);

    INPUT_RECORD record;
    DWORD read;

    if (!ReadConsoleInputW(hInput, &record, 1, &read) || read == 0)
        return InputEvent(InputModifier::None, InputKey::None, record.Event.KeyEvent.bKeyDown);

    if (record.EventType != KEY_EVENT)
        return InputEvent(InputModifier::None, InputKey::None, record.Event.KeyEvent.bKeyDown);

    const auto& keyEvent = record.Event.KeyEvent;
    const InputKey keyCode = static_cast<InputKey>(keyEvent.wVirtualKeyCode);
    const InputModifier modifiers = static_cast<InputModifier>(keyEvent.dwControlKeyState);
    const wchar_t unicodeChar = keyEvent.uChar.UnicodeChar;

    switch (keyCode)
    {
        case InputKey::UP:
        case InputKey::DOWN:
        case InputKey::LEFT:
        case InputKey::RIGHT:
        case InputKey::TAB:
        case InputKey::BACK:
        case InputKey::RETURN:
        case InputKey::SPACE:
        case InputKey::ESCAPE:
            return InputEvent(modifiers, keyCode, record.Event.KeyEvent.bKeyDown);
    }

    if (unicodeChar >= 32)
        return InputEvent(modifiers, keyCode, unicodeChar, record.Event.KeyEvent.bKeyDown);

    return InputEvent(InputModifier::None, InputKey::None, record.Event.KeyEvent.bKeyDown);
}

#endif // _WIN32
