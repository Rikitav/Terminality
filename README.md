# Terminality

Terminality is a C++23 TUI framework that brings WPF-style layout concepts to the terminal. It was built to replace manual cursor positioning with a proper widget hierarchy — you build the UI tree, and the framework handles the sizing, alignment, and redrawing.

## Quick start 🚀

```bash
git clone [https://github.com/Rikitav/Terminality.git](https://github.com/Rikitav/Terminality.git)
cd Terminality
mkdir build && cd build
cmake ..
cmake --build .

```

## Usage example

The layout is built using init-lambdas and smart pointers. Here’s a basic application with a centered text label and exit button :

```cpp
import terminality;
#include <terminality/Terminality.hpp>

using namespace terminality;

int main()
{
    auto& app = HostApplication::Current();
    app.EnterTerminal();

    auto root = init<StackPanel>([](auto* panel)
    {
        panel->HorizontalAlignment = HorizaontalAllign::Stretch;
        panel->VerticalAlignment = VerticalAlign::Stretch;
        
        panel->AddChild(init<Label>([](auto* label)
        {
            label->Text = L"Hello, Terminality!";
            label->HorizontalAlignment = HorizaontalAllign::Center;
            label->VerticalAlignment = VerticalAlign::Center;
        }));

        panel->AddChild(init<Button>([](Button* statusBar)
        {
            statusBar->Text = L"Exit";
            statusBar->HorizontalAlignment = HorizontalAlign::Stretch;
            statusBar->VerticalAlignment = VerticalAlign::Top;

            statusBar->Clicked += []()
            {
                HostApplication::Current().RequestStop();
            };
        }));
    });

    app.RunUILoop(std::move(root));
    app.ExitTerminal();
    
    return 0;
}

```

## Key features

* Widget hierarchy — containers (Grid, StackPanel, Border) and basic controls (Button, TextBox, CheckBox).
* Dynamic layout — automatic alignment and space distribution, familiar to anyone who has worked with XAML interfaces.
* Lazy rendering — the framework tracks dirty rects and only updates the specific console cells that have changed.
* Focus managment - widget focsuing and navigation are managed automatically, you dont need to write meshmaps.
* Multi-layered UI — built-in support for navigation, modal windows, context menus, and overlays on top of the main content.
* Event model — a signal and slot system for handling user input and binding shortcuts.
* Extensibility - you can write your own custom controls if you need speciffic behaviour or styling.

## License

MIT. Feel free to use it, break it, or embed it in your own projects without any restrictions.
