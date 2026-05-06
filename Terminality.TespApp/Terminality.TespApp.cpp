import std;
import std.compat;
import terminality;

#include <Windows.h>

using namespace terminality;

#define WITH(type, name, ...) \
    auto name = std::make_unique<type>(); \
    { auto& _ = *name; __VA_ARGS__; }

namespace
{
	class DemoRoot : public Grid
	{
		ObservableCollection<std::wstring> items_;

	public:
		DemoRoot()
		{
            // Настраиваем Grid на весь экран
            HorizontalAlignment = HorizontalAlignment::Stretch;
            VerticalAlignment = VerticalAlignment::Stretch;

            // 1. Делим гриду пополам вертикально (две колонки с весом Star)
            AddColumn(ColumnDefinition{ GridLength::Star(1.0f) });
            AddColumn(ColumnDefinition{ GridLength::Star(1.0f) });
            RegisterCombination(InputModifier::None, InputKey::ADD, []() { MessageBoxA(nullptr, "Test", nullptr, 0); });

            // 2. Создаем левую панель
            auto leftPanel = std::make_unique<StackPanel>();
            leftPanel->HorizontalAlignment = HorizontalAlignment::Stretch;

            for (int i = 1; i <= 3; ++i)
            {
                auto btn = ctor<Button>([&](Button* btn)
                {
                    btn->Text = (L"Left Panel Button " + std::to_wstring(i));
                    btn->HorizontalAlignment = HorizontalAlignment::Center;
                    btn->Margin = Thickness::Single;
                });

                leftPanel->AddChild(std::move(btn));
            }

            auto textBox = std::make_unique<TextBox>();
            textBox->Text = L"Type here...";
            textBox->Margin = Thickness::Single;
            textBox->MaxSize = Size(30, -1);
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

            items_.push_back(L"Dynamic Item 1");
            items_.push_back(L"Dynamic Item 2");
            items_.push_back(L"Dynamic Item 3");

            auto itemsControl = std::make_unique<ItemsControl<std::wstring>>();
            itemsControl->SetItemsSource(&items_);
            itemsControl->SetItemTemplate([](const std::wstring& item) -> std::unique_ptr<ControlBase> {
                auto btn = std::make_unique<Button>();
                btn->Text = item;
                btn->HorizontalAlignment = HorizontalAlignment::Center;
                return btn;
            });
            rightPanel->AddChild(std::move(itemsControl));

            auto addBtn = std::make_unique<Button>();
            addBtn->Text = L"Add new item";
            addBtn->HorizontalAlignment = HorizontalAlignment::Center;
            addBtn->Margin = Thickness::Single;
            addBtn->Clicked += [this]() {
                items_.push_back(L"New Dynamic Item " + std::to_wstring(items_.size() + 1));
            };
            rightPanel->AddChild(std::move(addBtn));

            auto removeBtn = std::make_unique<Button>();
            removeBtn->Text = L"Remove last item";
            removeBtn->HorizontalAlignment = HorizontalAlignment::Center;
            removeBtn->Clicked += [this]() {
                if (!items_.empty())
                    items_.pop_back();
            };
            rightPanel->AddChild(std::move(removeBtn));

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
