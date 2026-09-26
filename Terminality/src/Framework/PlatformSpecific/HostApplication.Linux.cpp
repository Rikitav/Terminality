#if defined(__linux__) || defined(__APPLE__)

#include <cstdlib>
#include <iostream>

#include <terminality/Framework/HostApplication.hpp>
#include <terminality/Core/InputEvent.hpp>

#include <sys/ioctl.h>
#include <clocale>
#include <termios.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <cwchar>
#include <string>

using namespace terminality;

namespace
{
	struct WakePipe
	{
		int readFd = -1;
		int writeFd = -1;
	};

	WakePipe& GetWakePipe()
	{
		static WakePipe pipe = []
		{
			WakePipe p;
			int fds[2];
			if (::pipe(fds) == 0)
			{
				fcntl(fds[0], F_SETFL, O_NONBLOCK);
				fcntl(fds[1], F_SETFL, O_NONBLOCK);
				p.readFd = fds[0];
				p.writeFd = fds[1];
			}

			return p;
		}();

		return pipe;
	}
}

static struct termios original_termios;

namespace
{
	static InputKey CharToInputKey(wchar_t ch)
	{
		if (ch >= L'a' && ch <= L'z')
			return static_cast<InputKey>(static_cast<int>(InputKey::A) + (ch - L'a'));
		
		if (ch >= L'A' && ch <= L'Z')
			return static_cast<InputKey>(static_cast<int>(InputKey::A) + (ch - L'A'));
		
		if (ch >= L'0' && ch <= L'9')
			return static_cast<InputKey>(static_cast<int>(InputKey::NUM0) + (ch - L'0'));
		
		if (ch == L' ')
			return InputKey::SPACE;

		return InputKey::CHAR;
	}

	static InputEvent MakeCharEvent(wchar_t ch)
	{
		InputKey key = CharToInputKey(ch);
		if (key == InputKey::CHAR)
			return InputEvent(ch, true);
		
		return InputEvent(InputModifier::None, key, ch, true);
	}
}

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

void HostBackend::SignalInput()
{
	const WakePipe& p = GetWakePipe();
	if (p.writeFd >= 0)
	{
		const char byte = 1;
		ssize_t result = write(p.writeFd, &byte, 1);
		(void)result; // EAGAIN when the pipe is full: the signal is already pending
	}
}

void HostBackend::ResetInputSignal()
{
	const WakePipe& p = GetWakePipe();
	if (p.readFd < 0)
		return;

	char buffer[64];
	while (true)
	{
		ssize_t result = read(p.readFd, buffer, sizeof(buffer));
		if (result <= 0)
			break;
	}
}

InputEvent HostBackend::PollInput(std::chrono::milliseconds timeout)
{
	static std::string pending;

	const int wakeReadFd = GetWakePipe().readFd;
	struct pollfd fds[2] = {
		{ STDIN_FILENO, POLLIN, 0 },
		{ wakeReadFd, POLLIN, 0 }
	};

	const nfds_t nfds = wakeReadFd >= 0 ? 2 : 1;

	// If we already have buffered bytes, only do a non-blocking check for more.
	int pollTimeout = pending.empty() ? static_cast<int>(timeout.count()) : 0;
	int ret = poll(fds, nfds, pollTimeout);

	if (ret > 0 && nfds == 2 && (fds[1].revents & POLLIN))
		ResetInputSignal();

	if (ret > 0 && (fds[0].revents & POLLIN))
	{
		char buffer[256];
		ssize_t bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer));

		if (bytesRead > 0)
			pending.append(buffer, static_cast<std::size_t>(bytesRead));
	}

	if (pending.empty())
		return InputEvent(InputModifier::None, InputKey::None, false);

	auto consume = [&](std::size_t n, InputEvent evt)
	{
		pending.erase(0, n);
		return evt;
	};

	// Escape sequences.
	if (pending[0] == '\x1b')
	{
		if (pending.size() == 1)
			return consume(1, InputEvent(InputModifier::None, InputKey::ESCAPE, true));

		if (pending[1] == '[')
		{
			if (pending.size() >= 3)
			{
				switch (pending[2])
				{
					case 'A': return consume(3, InputEvent(InputModifier::None, InputKey::UP, true));
					case 'B': return consume(3, InputEvent(InputModifier::None, InputKey::DOWN, true));
					case 'C': return consume(3, InputEvent(InputModifier::None, InputKey::RIGHT, true));
					case 'D': return consume(3, InputEvent(InputModifier::None, InputKey::LEFT, true));
					case 'H': return consume(3, InputEvent(InputModifier::None, InputKey::HOME, true));
					case 'F': return consume(3, InputEvent(InputModifier::None, InputKey::END, true));
				}

				// CSI numeric sequences terminated by '~' (Insert, Delete, PgUp, PgDn, Home, End).
				std::size_t tilde = pending.find('~', 2);
				if (tilde != std::string::npos)
				{
					std::string seq = pending.substr(2, tilde - 2);
					InputKey key = InputKey::None;

					if (seq.size() == 1)

					switch (seq[0])
					{
						case '2': return consume(tilde + 1, InputEvent(InputModifier::None, InputKey::INSERT, true));
						case '3': return consume(tilde + 1, InputEvent(InputModifier::None, InputKey::DELETE, true));
						case '5': return consume(tilde + 1, InputEvent(InputModifier::None, InputKey::PRIOR, true));
						case '6': return consume(tilde + 1, InputEvent(InputModifier::None, InputKey::NEXT, true));
						case '7':
						case '1': return consume(tilde + 1, InputEvent(InputModifier::None, InputKey::HOME, true));
						case '8':
						case '4': return consume(tilde + 1, InputEvent(InputModifier::None, InputKey::END, true));
					}
				}
			}
		}
		else if (pending[1] == 'O')
		{
			if (pending.size() >= 3)
			{
				switch (pending[2])
				{
					case 'P': return consume(3, InputEvent(InputModifier::None, InputKey::F1, true));
					case 'Q': return consume(3, InputEvent(InputModifier::None, InputKey::F2, true));
					case 'R': return consume(3, InputEvent(InputModifier::None, InputKey::F3, true));
					case 'S': return consume(3, InputEvent(InputModifier::None, InputKey::F4, true));
					case 'H': return consume(3, InputEvent(InputModifier::None, InputKey::HOME, true));
					case 'F': return consume(3, InputEvent(InputModifier::None, InputKey::END, true));
				}
			}
		}
		else
		{
			// Alt+key sequence: ESC followed by a control or printable byte.
			unsigned char c = static_cast<unsigned char>(pending[1]);
			switch (c)
			{
				case '\t': return consume(2, InputEvent(InputModifier::Alt, InputKey::TAB, true));
				case '\n':
				case '\r': return consume(2, InputEvent(InputModifier::Alt, InputKey::RETURN, true));
				case 127:
				case '\b': return consume(2, InputEvent(InputModifier::Alt, InputKey::BACK, true));
				case ' ':  return consume(2, InputEvent(InputModifier::Alt, InputKey::SPACE, true));
			}

			if (c >= 1 && c <= 26)
			{
				InputKey key = static_cast<InputKey>(static_cast<int>(InputKey::A) + (c - 1));
				return consume(2, InputEvent(InputModifier::Alt | InputModifier::Ctrl, key, true));
			}
			if (c >= 32 && c < 127)
			{
				wchar_t ch = static_cast<wchar_t>(c);
				InputKey key = CharToInputKey(ch);
				if (key == InputKey::CHAR)
					return consume(2, InputEvent(InputModifier::Alt, ch, true));

				return consume(2, InputEvent(InputModifier::Alt, key, ch, true));
			}
		}

		// Unknown or incomplete escape sequence: emit ESC and let the following bytes be interpreted on the next call.
		return consume(1, InputEvent(InputModifier::None, InputKey::ESCAPE, true));
	}

	unsigned char c = static_cast<unsigned char>(pending[0]);
	switch (c)
	{
		case '\t': return consume(2, InputEvent(InputModifier::None, InputKey::TAB, true));
		case '\n':
		case '\r': return consume(2, InputEvent(InputModifier::None, InputKey::RETURN, true));
		case 127:
		case '\b': return consume(2, InputEvent(InputModifier::None, InputKey::BACK, true));
		case ' ':  return consume(2, InputEvent(InputModifier::None, InputKey::SPACE, true));
	}

	// Ctrl+letter: bytes 1-26 map to A-Z.
	if (c >= 1 && c <= 26)
	{
		InputKey key = static_cast<InputKey>(static_cast<int>(InputKey::A) + (c - 1));
		return consume(1, InputEvent(InputModifier::Ctrl, key, true));
	}

	// UTF-8 character.
	std::mbstate_t state{};
	wchar_t wc = 0;
	std::size_t len = std::mbrtowc(&wc, pending.c_str(), pending.size(), &state);
	if (len > 0 && len != static_cast<std::size_t>(-1) && len != static_cast<std::size_t>(-2) && wc >= 32)
		return consume(len, MakeCharEvent(wc));

	if (len == static_cast<std::size_t>(-2))
	{
		// Incomplete UTF-8 sequence: wait for more bytes.
		return InputEvent(InputModifier::None, InputKey::None, false);
	}

	// Unknown byte; drop it.
	return consume(1, InputEvent(InputModifier::None, InputKey::None, false));
}

#endif // __linux__ || __APPLE__
