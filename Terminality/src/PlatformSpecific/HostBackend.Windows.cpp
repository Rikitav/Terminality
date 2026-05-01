module;

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

module terminality;

import std;

using namespace terminality;

#ifdef _WIN32
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
        return InputEvent(unicodeChar, record.Event.KeyEvent.bKeyDown);

    return InputEvent(InputModifier::None, InputKey::None, record.Event.KeyEvent.bKeyDown);
}

#endif // _WIN32
