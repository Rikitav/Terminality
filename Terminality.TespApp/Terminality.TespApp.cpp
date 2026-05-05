import std;
import std.compat;
import terminality;

#include <Windows.h>

using namespace terminality;

namespace
{
	class DemoRoot : public Grid
	{
		EventSignalConnection<>* buttonClicked_ = nullptr;

	public:
		DemoRoot()
		{
            // Настраиваем Grid на весь экран
            HorizontalAlignment = HorizontalAlignment::Stretch;
            VerticalAlignment = VerticalAlignment::Stretch;

            // 1. Делим гриду пополам вертикально (две колонки с весом Star)
            AddColumn(ColumnDefinition{ GridLength::Star(1.0f) });
            AddColumn(ColumnDefinition{ GridLength::Star(1.0f) });

            // 2. Создаем левую панель
            auto leftPanel = std::make_unique<StackPanel>();
            leftPanel->HorizontalAlignment = HorizontalAlignment::Stretch;

            for (int i = 1; i <= 3; ++i)
            {
                auto btn = std::make_unique<Button>();
                btn->Text = (L"Left Panel Button " + std::to_wstring(i));
                btn->HorizontalAlignment = HorizontalAlignment::Center;
                btn->Margin = Thickness::Single;

                leftPanel->AddChild(std::move(btn));
            }

            auto textBox = std::make_unique<TextBox>();
            textBox->Text = L"Type here...";
            textBox->Margin = Thickness::Single;
            textBox->MaxSize = Size(25, -1);
            textBox->HorizontalAlignment = HorizontalAlignment::Stretch;
            textBox->VerticalAlignment = VerticalAlignment::Center;
            textBox->AcceptsReturn = true;
            leftPanel->AddChild(std::move(textBox));

            auto leftBorder = std::make_unique<Border>();
			leftBorder->BorderColor = Color::YELLOW;
			leftBorder->Content = std::move(leftPanel);
            leftBorder->HorizontalAlignment = HorizontalAlignment::Center;
            AddChild(std::move(leftBorder), 0, 0);

            // 3. Создаем правую панель
            auto rightPanel = std::make_unique<StackPanel>();
            rightPanel->HorizontalAlignment = HorizontalAlignment::Stretch;

            for (int i = 1; i <= 3; ++i)
            {
                auto btn = std::make_unique<Button>();
                btn->Text = L"Right Panel Button " + std::to_wstring(i);
                btn->HorizontalAlignment = HorizontalAlignment::Center;
                btn->Clicked += []() { MessageBoxA(nullptr, "Test", nullptr, 0); };

                rightPanel->AddChild(std::move(btn));
            }

            auto checkBox = std::make_unique<CheckBox>();
            checkBox->Text = L"Check box";
            checkBox->HorizontalAlignment = HorizontalAlignment::Center;
            rightPanel->AddChild(std::move(checkBox));

            auto rightBorder = std::make_unique<Border>();
            rightBorder->BorderColor = Color::YELLOW;
            rightBorder->Content = std::move(rightPanel);
            AddChild(std::move(rightBorder), 0, 1);
		}
	};
}

int main()
{
	HostApplication& app = HostApplication::Current();
	app.EnterTerminal();
    app.SetRoot(std::make_unique<DemoRoot>());
    app.RunUILoop();
	app.ExitTerminal();
	return 0;
}
