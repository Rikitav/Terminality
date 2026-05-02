import std;
import std.compat;
import terminality;

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
            SetHorizontalAlignment(HorizontalAlignment::Stretch);
            SetVerticalAlignment(VerticalAlignment::Stretch);

            // 1. Делим гриду пополам вертикально (две колонки с весом Star)
            AddColumn(ColumnDefinition{ GridLength::Star(1.0f) });
            AddColumn(ColumnDefinition{ GridLength::Star(1.0f) });

            // 2. Создаем левую панель
            auto leftPanel = std::make_unique<StackPanel>();
            leftPanel->SetHorizontalAlignment(HorizontalAlignment::Stretch);

            for (int i = 1; i <= 3; ++i)
            {
                auto btn = std::make_unique<Button>();
                btn->SetText(L"Left Panel Button " + std::to_wstring(i));
                btn->SetHorizontalAlignment(HorizontalAlignment::Center);
                btn->SetMargin(Thickness(1));
                leftPanel->AddChild(std::move(btn));
            }

            auto textBox = std::make_unique<TextBox>();
            textBox->SetText(L"Type here...");
            textBox->SetMargin(Thickness(1));
            leftPanel->AddChild(std::move(textBox));

            // Добавляем левую панель в первую колонку (0, 0)
			auto leftBorder = std::make_unique<Border>();
			leftBorder->SetBorderColor(Color::YELLOW);
			leftBorder->SetContent(std::move(leftPanel));
            leftBorder->SetHorizontalAlignment(HorizontalAlignment::Center);
            AddChild(std::move(leftBorder), 0, 0);

            // 3. Создаем правую панель
            auto rightPanel = std::make_unique<StackPanel>();
            rightPanel->SetHorizontalAlignment(HorizontalAlignment::Stretch);

            for (int i = 1; i <= 3; ++i)
            {
                auto btn = std::make_unique<Button>();
                btn->SetText(L"Right Panel Button " + std::to_wstring(i));
                btn->SetHorizontalAlignment(HorizontalAlignment::Center);
                rightPanel->AddChild(std::move(btn));
            }

            auto rightBorder = std::make_unique<Border>();
            rightBorder->SetBorderColor(Color::YELLOW);
            rightBorder->SetContent(std::move(rightPanel));
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
