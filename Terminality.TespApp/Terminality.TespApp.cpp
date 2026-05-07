import std;
import std.compat;
import terminality;

using namespace terminality;

namespace
{
    class MessangerTest : public Grid
    {
        ObservableCollection<std::wstring> chatHistory_;

    public:
        MessangerTest()
        {
            // Растягиваем корневую сетку на весь экран
            HorizontalAlignment = HorizontalAlignment::Stretch;
            VerticalAlignment = VerticalAlignment::Stretch;

            // Определяем 3 строки (предполагается, что методы AddRow и Auto() существуют по аналогии с колонками)
            AddRow(RowDefinition{ GridLength::Star(1.0f) }); // Строка 0: Основной чат
            AddRow(RowDefinition{ GridLength::Pixel(3) });     // Строка 1: Поле ввода
            AddRow(RowDefinition{ GridLength::Pixel(2) });   // Строка 2: Футер (подсказки)

            // ==========================================
            // 1. ИСТОРИЯ ЧАТА (Строка 0)
            // ==========================================
            chatHistory_.push_back(L"[04:12:04] 123");
            chatHistory_.push_back(L"[04:00:00] : Переключено в чат: Tamerlan");
            chatHistory_.push_back(L"[04:13:42] Привет!");
            chatHistory_.push_back(L"[04:13:51] Tamerlan: Hello, World!");

            AddChild(0, 0, ctor<Border>([&](Border* chatBorder)
            {
                chatBorder->HeaderText = L"Чат: Tamerlan";
                chatBorder->Content = ctor<ItemsControl<std::wstring>>([&](ItemsControl<std::wstring>*chatList)
                {
                    chatList->SetItemsSource(&chatHistory_);
                    chatList->SetItemTemplate([](const std::wstring& item) -> std::unique_ptr<ControlBase>
                    {
                        auto msg = std::make_unique<TextBox>();
                        msg->HorizontalAlignment = HorizontalAlignment::Stretch;
                        msg->Text = item;
                        return msg;
                    });
                });
            }));

            // ==========================================
            // 2. ПАНЕЛЬ ВВОДА (Строка 1)
            // ==========================================
            AddChild(1, 0, std::make_unique<Border>(ctor<Grid>([&](Grid* inputGrid)
            {
                inputGrid->HorizontalAlignment = HorizontalAlignment::Stretch;
                inputGrid->AddColumn(ColumnDefinition{ GridLength::Auto() });     // Для префикса "Rikitav@Tamerlan>"
                inputGrid->AddColumn(ColumnDefinition{ GridLength::Star(1.0f) }); // Для поля ввода сообщения

                // Префикс командной строки
                inputGrid->AddChild(0, 0, ctor<Button>([&](Button* promptLabel)
                {
                    promptLabel->Text = L"Rikitav@Tamerlan> ";
                    promptLabel->MaxSize = Size(-1, 1);
                    promptLabel->HorizontalAlignment = HorizontalAlignment::Left;
                }));

                // Поле для ввода текста
                inputGrid->AddChild(0, 1, ctor<TextBox>([&](TextBox* inputBox)
                {
                    inputBox->Text = L"";
                    inputBox->MaxSize = Size(-1, 1);
                    inputBox->HorizontalAlignment = HorizontalAlignment::Stretch;
                    inputBox->AcceptsReturn = false;
                    inputBox->FocusedBackgroundColor = Color::BLACK;
                    inputBox->FocusedForegroundColor = Color::WHITE;

                    inputBox->RegisterCombination(InputModifier::None, InputKey::ADD, [&](ControlBase* self)
                    {
                        TextBox* selfTextBox = static_cast<TextBox*>(self);
                        this->chatHistory_.push_back(selfTextBox->Text);
                        selfTextBox->Text = L"";
                    });
                }));
            })));
            
            // ==========================================
            // 3. СТАТУС-БАР (Строка 2)
            // ==========================================
            AddChild(2, 0, ctor<Button>([&](Button* statusBar)
            {
                statusBar->Text = L"ESC - выход | /help | Чат: Tamerlan";
                statusBar->HorizontalAlignment = HorizontalAlignment::Stretch;
            }));

            // ==========================================
            // ГОРЯЧИЕ КЛАВИШИ
            // ==========================================
            RegisterCombination(InputModifier::None, InputKey::ESCAPE, [](ControlBase* self)
            {
                HostApplication::Current().RequestStop();
            });
        }
    };
}

int main()
{
	HostApplication& app = HostApplication::Current();
	app.EnterTerminal();
    app.SetRoot(std::make_unique<MessangerTest>());
    app.RunUILoop();
	app.ExitTerminal();
	return 0;
}
