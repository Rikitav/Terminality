#include "Common.hpp"
#include "MainWindow.hpp"

using namespace terminality;

int main()
{
	RenderBuffer::TrueColorOutput = false;

	HostApplication& app = HostApplication::Current();
	app.EnterTerminal();

	SetConsoleTitleW(L"btop++ (Terminality clone)");

	auto root = std::make_unique<btop::MainWindow>();
	app.RunUILoop(std::move(root));
	app.ExitTerminal();

	return 0;
}
