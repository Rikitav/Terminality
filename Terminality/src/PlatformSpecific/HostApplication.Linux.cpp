module;

#if defined(__linux__) || defined(__APPLE__)
#include <sys/ioctl.h>
#include <clocale>
#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <termios.h>
#include <cstdlib>
#endif // __linux__ || __APPLE__

module terminality;

import std;
import std.compat;

using namespace terminality;

#if defined(__linux__) || defined(__APPLE__)

static struct termios original_termios;

void HostApplication::EnterTerminal()
{
	std::setlocale(LC_ALL, "");

	try
	{
		std::locale loc("");
		std::locale::global(loc);
		std::cout.imbue(loc);
		std::cerr.imbue(loc);
		std::wcout.imbue(loc);
		std::wcin.imbue(loc);
	}
	catch (...)
	{
		std::cerr << "Warning: Failed to set UTF-8 locale. UI may render incorrectly.\n";
	}

	tcgetattr(STDIN_FILENO, &original_termios);

	struct termios raw = original_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

	std::ios_base::sync_with_stdio(false);
	std::wcout.tie(nullptr);

	std::wcout << L"\x1b[?1049h\x1b[0m\x1b[40m\x1b[?7l\x1b[2J\x1b[?25l";
	std::wcout.flush();
}

void HostApplication::ExitTerminal()
{
	std::wcout << L"\x1b[?1049l\x1b[?7h\x1b[?25h";
	std::wcout.flush();

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

Size HostBackend::QueryViewportSize()
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return Size(w.ws_col, w.ws_row);
}

InputEvent HostBackend::PollInput(std::chrono::milliseconds timeout)
{
	struct pollfd pfd = { STDIN_FILENO, POLLIN, 0 };
	int ret = poll(&pfd, 1, timeout.count());

	if (ret <= 0 || !(pfd.revents & POLLIN))
		return InputEvent(InputModifier::None, InputKey::None, false);

	char buffer[32];
	ssize_t bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer) - 1);

	if (bytesRead <= 0)
		return InputEvent(InputModifier::None, InputKey::None, false);

	buffer[bytesRead] = '\0';

	if (buffer[0] == '\x1b')
	{
		if (bytesRead == 1)
			return InputEvent(InputModifier::None, InputKey::ESCAPE, true);

		if (bytesRead >= 3 && buffer[1] == '[')
		{
			switch (buffer[2])
			{
				case 'A': return InputEvent(InputModifier::None, InputKey::UP, true);
				case 'B': return InputEvent(InputModifier::None, InputKey::DOWN, true);
				case 'C': return InputEvent(InputModifier::None, InputKey::RIGHT, true);
				case 'D': return InputEvent(InputModifier::None, InputKey::LEFT, true);
			}
		}
		return InputEvent(InputModifier::None, InputKey::None, false);
	}

	if (bytesRead == 1)
	{
		char c = buffer[0];
		if (c == '\t') return InputEvent(InputModifier::None, InputKey::TAB, true);
		if (c == '\n' || c == '\r') return InputEvent(InputModifier::None, InputKey::RETURN, true);
		if (c == 127 || c == '\b') return InputEvent(InputModifier::None, InputKey::BACK, true);
		if (c == ' ') return InputEvent(InputModifier::None, InputKey::SPACE, true);
	}

	wchar_t wc = 0;
	if (std::mbtowc(&wc, buffer, bytesRead) > 0 && wc >= 32)
		return InputEvent(wc, true);

	return InputEvent(InputModifier::None, InputKey::None, false);
}

#endif // __linux__ || __APPLE__
