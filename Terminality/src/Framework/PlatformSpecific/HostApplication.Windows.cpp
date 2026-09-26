#ifdef _WIN32

#include <algorithm>
#include <array>
#include <chrono>
#include <clocale>
#include <cstdint>
#include <deque>
#include <iostream>
#include <string>
#include <thread>
#include <memory>
#include <vector>

#include <terminality/Framework/HostApplication.hpp>
#include <Windows.h>

#ifdef TRANSPARENT
#undef TRANSPARENT
#endif

#ifdef MessageBox
#undef MessageBox
#endif

namespace
{
	HANDLE GetWakeEvent()
	{
		static HANDLE event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
		return event;
	}
}

using namespace terminality;

namespace
{
    // Classic conhost never delivers a distinct key-up record
    // for a character key pressed together with Shift/Ctrl/Alt
    // 
    // (e.g. Shift+5 -> '%'): presses stream as key-down records only (auto-repeat included), and a single
    // VK_SHIFT/VK_CONTROL/VK_MENU key-up stands in for every eaten release.
    // 
    // Two mechanisms reconstruct the missing key-ups:
    // 
    // - re-press settlement: a key-down arriving after a silent gap longer
    //    than auto-repeat intervals means the key was released and pressed
    //    again, so the previous press is settled with a synthesized key-up;
    // 
    // - modifier drain: when the modifier is released, every still-tracked
    //    key down that carried it is settled the same way.
    // 
    // Synthesized events go through a queue because one input record can owe several,
    // and the matching real key-up records (which do arrive for unshifted keys) are swallowed to avoid duplicates.
    struct TrackedKey
    {
        InputKey Key;
        InputModifier Mods;
        unsigned Count;
        unsigned SwallowUps;
        std::chrono::steady_clock::time_point LastDown;
    };

    constexpr std::chrono::milliseconds RepressThreshold(200);

    std::vector<TrackedKey>& TrackedKeys()
    {
        static std::vector<TrackedKey> keys;
        return keys;
    }

    std::deque<InputEvent>& PendingEvents()
    {
        static std::deque<InputEvent> events;
        return events;
    }

    std::array<bool, 3>& ModifierDownState()
    {
        static std::array<bool, 3> state{};
        return state;
    }

    bool HasMod(InputModifier mods, InputModifier flag)
    {
        return (static_cast<uint32_t>(mods) & static_cast<uint32_t>(flag)) != 0;
    }

    InputModifier WithoutGroup(InputModifier mods, InputModifier group)
    {
        return static_cast<InputModifier>(static_cast<uint32_t>(mods) & ~static_cast<uint32_t>(group));
    }

    InputEvent TrackKeyDown(InputKey key, InputModifier mods, unsigned count, InputEvent downEvent)
    {
        if (key == InputKey::None)
            return downEvent;

        std::vector<TrackedKey>& keys = TrackedKeys();
        const auto now = std::chrono::steady_clock::now();
        
        const auto it = std::find_if(
            keys.begin(), keys.end(),
            [key](const TrackedKey& k) { return k.Key == key; });

        if (it != keys.end() && now - it->LastDown >= RepressThreshold)
        {
            std::deque<InputEvent>& pending = PendingEvents();
            InputEvent settleUp(it->Mods, it->Key, false);

            for (unsigned i = 0; i < count; ++i)
                pending.push_back(downEvent);

            // If the eaten real key-up record ever arrives, swallow it.
            it->Mods = mods;
            it->Count = count;
            it->SwallowUps++;
            it->LastDown = now;
            return settleUp;
        }

        if (it == keys.end())
            keys.push_back({ key, mods, count, 0, now });

        else
        {
            it->Count += count;
            it->LastDown = now;
        }

        return downEvent;
    }

    InputEvent UntrackKey(InputKey key, InputEvent realUp)
    {
        std::vector<TrackedKey>& keys = TrackedKeys();
        const auto it = std::find_if(
            keys.begin(), keys.end(),
            [key](const TrackedKey& k) { return k.Key == key; });
        
        if (it == keys.end())
            return realUp;

        if (it->SwallowUps > 0)
        {
            it->SwallowUps--;
            return InputEvent(InputModifier::None, InputKey::None, false);
        }

        keys.erase(it);
        return realUp;
    }

    bool IsModifierKey(InputKey key)
    {
        switch (key)
        {
            case InputKey::LSHIFT:
            case InputKey::RSHIFT:
            case InputKey::SHIFT:
            case InputKey::LCONTROL:
            case InputKey::RCONTROL:
            case InputKey::CONTROL:
            case InputKey::LMENU:
            case InputKey::RMENU:
            case InputKey::MENU:
            case InputKey::LWIN:
            case InputKey::RWIN:
                return true;
            
            default:
                return false;
        }
    }

    InputModifier ModifierGroupFor(InputKey key)
    {
        switch (key)
        {
            case InputKey::LSHIFT:
            case InputKey::RSHIFT:
            case InputKey::SHIFT:
                return InputModifier::Shift;
            
            case InputKey::LCONTROL:
            case InputKey::RCONTROL:
            case InputKey::CONTROL:
                return InputModifier::Ctrl;
            
            case InputKey::LMENU:
            case InputKey::RMENU:
            case InputKey::MENU:
                return InputModifier::Alt;

            default:
                return InputModifier::None;
        }
    }

    int ModifierGroupIndex(InputModifier group)
    {
        if (group == InputModifier::Shift)
            return 0;

        if (group == InputModifier::Ctrl)
            return 1;

        if (group == InputModifier::Alt)
            return 2;

        return -1;
    }

    void DrainKeysFor(InputKey modifierKey)
    {
        const InputModifier group = ModifierGroupFor(modifierKey);
        if (group == InputModifier::None)
            return;

        std::vector<TrackedKey>& keys = TrackedKeys();
        std::deque<InputEvent>& pending = PendingEvents();
        const uint32_t groupBits = static_cast<uint32_t>(group);

        for (const TrackedKey& k : keys)
        {
            // Any bit of the group matches: LeftCtrl alone counts as Ctrl.
            if ((static_cast<uint32_t>(k.Mods) & groupBits) == 0)
                continue;

            for (unsigned i = 0; i < k.Count; ++i)
                pending.emplace_back(k.Mods, k.Key, false);
        }

        keys.erase(std::remove_if(
            keys.begin(), keys.end(),
            [groupBits](const TrackedKey& k) { return (static_cast<uint32_t>(k.Mods) & groupBits) != 0; }), keys.end());
    }
}

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

    SetConsoleCtrlHandler([](DWORD) -> BOOL
    {
        HostApplication::Current().ExitTerminal();
        ExitProcess(0);
        return TRUE;
    }, TRUE);

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

void HostBackend::SignalInput()
{
    SetEvent(GetWakeEvent());
}

void HostBackend::ResetInputSignal()
{
    ResetEvent(GetWakeEvent());
}

InputEvent HostBackend::PollInput(std::chrono::milliseconds timeout)
{
    // Synthesized key-ups are returned before reading new input
    std::deque<InputEvent>& pending = PendingEvents();
    if (!pending.empty())
    {
        InputEvent event = pending.front();
        pending.pop_front();
        return event;
    }

    static HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    static HANDLE hWake = GetWakeEvent();

    const HANDLE handles[2] = { hInput, hWake };
    DWORD waitResult = WaitForMultipleObjects(2, handles, FALSE, static_cast<DWORD>(timeout.count()));

    if (waitResult == WAIT_OBJECT_0 + 1)
        return InputEvent(InputModifier::None, InputKey::None, false);

    if (waitResult != WAIT_OBJECT_0)
        return InputEvent(InputModifier::None, InputKey::None, false);

    INPUT_RECORD record;
    DWORD read;

    if (!ReadConsoleInputW(hInput, &record, 1, &read) || read == 0)
        return InputEvent(InputModifier::None, InputKey::None, false);

    if (record.EventType != KEY_EVENT)
        return InputEvent(InputModifier::None, InputKey::None, false);

    const auto& keyEvent = record.Event.KeyEvent;
    const InputKey keyCode = static_cast<InputKey>(keyEvent.wVirtualKeyCode);
    const InputModifier modifiers = static_cast<InputModifier>(keyEvent.dwControlKeyState);
    const wchar_t unicodeChar = keyEvent.uChar.UnicodeChar;
    const bool pressed = keyEvent.bKeyDown;
    const unsigned repeatCount = keyEvent.wRepeatCount > 0 ? keyEvent.wRepeatCount : 1;

    // A pressed record owes one event per repeat
    auto expandPressed = [&](InputEvent event) -> InputEvent
    {
        for (unsigned i = 1; i < repeatCount; ++i)
            pending.push_back(event);

        return event;
    };

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
        {
            return pressed
                ? expandPressed(InputEvent(modifiers, keyCode, true))
                : InputEvent(modifiers, keyCode, false);
        }
    }

    if (unicodeChar >= 32)
    {
        if (pressed)
        {
            InputEvent down(modifiers, keyCode, unicodeChar, true);
            InputEvent deliver = TrackKeyDown(keyCode, modifiers, repeatCount, down);
            if (deliver.Pressed)
            {
                // No re-press settlement
                for (unsigned i = 1; i < repeatCount; ++i)
                    pending.push_back(down);
            }

            return deliver;
        }

        return UntrackKey(keyCode, InputEvent(modifiers, keyCode, unicodeChar, false));
    }

    // Modifier keys are routed as plain key events.
    // Physical state is deduped so auto-repeat and the phantom records do not produce spurious events
    if (IsModifierKey(keyCode))
    {
        const InputModifier group = ModifierGroupFor(keyCode);
        const int groupIndex = ModifierGroupIndex(group);
        std::array<bool, 3>& modState = ModifierDownState();
        const InputModifier ownMods = group == InputModifier::None
            ? modifiers
            : WithoutGroup(modifiers, group);

        if (pressed)
        {
            if (groupIndex >= 0 && modState[groupIndex])
                return InputEvent(InputModifier::None, InputKey::None, true);

            if (groupIndex >= 0)
                modState[groupIndex] = true;

            return InputEvent(ownMods, keyCode, true);
        }

        const bool wasDown = groupIndex < 0 || modState[groupIndex];
        if (groupIndex >= 0)
            modState[groupIndex] = false;

        // Settle tracked keys first
        DrainKeysFor(keyCode);

        InputEvent up(ownMods, keyCode, false);
        if (!pending.empty())
        {
            pending.push_back(up);
            InputEvent event = pending.front();
            pending.pop_front();
            return event;
        }

        // Nothing to settle
        if (!wasDown)
            return InputEvent(InputModifier::None, InputKey::None, false);

        return up;
    }

    if (pressed)
    {
        InputEvent down(modifiers, keyCode, true);
        InputEvent deliver = TrackKeyDown(keyCode, modifiers, repeatCount, down);
        if (deliver.Pressed)
        {
            for (unsigned i = 1; i < repeatCount; ++i)
                pending.push_back(down);
        }

        return deliver;
    }

    return UntrackKey(keyCode, InputEvent(modifiers, keyCode, false));
}

void terminality::AlertAsync(const std::wstring& text, const std::wstring& title)
{
    std::thread([text, title]() { MessageBoxW(nullptr, text.c_str(), title.size() == 0 ? nullptr : title.c_str(), MB_OK | MB_ICONINFORMATION); }).detach();
}

#endif // _WIN32
