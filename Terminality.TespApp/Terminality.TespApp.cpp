import std;
import std.compat;
import terminality;

//#include <Windows.h>

#undef MessageBox

using namespace terminality;

struct MessageModel
{
    bool isAuthor;
    std::wstring Timestamp;
    std::wstring Text;
};

namespace TestApp
{
    class MessageBubble : public ControlBase
    {
    public:
        MessageModel message_;

        MessageBubble()
        {
            HorizontalAlignment = HorizaontalAllign::Left;
        }

        Size MeasureOverride(const Size& availableSize) override
        {
            static const int timeStampLength = 10;
            return Size(timeStampLength + static_cast<int>(message_.Text.size()) + 1, 1);
        }

        void ArrangeOverride(const Rect& finalRect) override
        {
            return;
        }
        
        void RenderOverride(RenderContext& context) override
        {
            auto rin = context.BeginText();
            if (focused_)
            {
                rin << SetBack(Color::WHITE);
                rin << SetFore(Color::DARK_GRAY) << message_.Timestamp;
            }
            else
            {
                rin << SetBack(Color::BLACK);
                rin << SetFore(Color::LIGHT_GRAY) << message_.Timestamp;
            }

            rin << SetFore(message_.isAuthor ? Color::CYAN : Color::YELLOW);
            rin << " " << message_.Text;
        }
    };

    class MessangerTest : public Grid
    {
        ObservableCollection<MessageModel> chatHistory_;

    public:
        MessangerTest()
        {
            // Растягиваем корневую сетку на весь экран
            HorizontalAlignment = HorizaontalAllign::Stretch;
            VerticalAlignment = VerticalAlign::Stretch;

            AddRow(RowDefinition{ GridLength::Star(1.0f) });   // Строка 0: Основной чат
            AddRow(RowDefinition{ GridLength::Cell(3) });      // Строка 1: Поле ввода
            AddRow(RowDefinition{ GridLength::Cell(2) });      // Строка 2: Футер (подсказки)

            // ==========================================
            // 1. ИСТОРИЯ ЧАТА (Строка 0)
            // ==========================================
            chatHistory_.push_back(MessageModel { true, L"[04:12:04]", L"123" });
            chatHistory_.push_back(MessageModel { false, L"[04:13:42]", L"Привет!" });
            chatHistory_.push_back(MessageModel { true, L"[04:13:51]", L"Tamerlan: Hello, World!" });

            AddChild(0, 0, init<Border>([&](Border* chatBorder)
            {
                chatBorder->HeaderText = L"Чат: Tamerlan";
                chatBorder->Content = init<ItemsControl<MessageModel>>([&](ItemsControl<MessageModel>* chatList)
                {
                    //chatList->ContentOrientation = Orientation::Horizontal;
                    chatList->SetItemsSource(&chatHistory_);

                    chatList->SetItemTemplate([](const MessageModel& item) -> std::unique_ptr<ControlBase>
                    {
                        return init<MessageBubble>([&](MessageBubble* bubble)
                        {
                            bubble->message_ = item;
                            bubble->CtxMenu = init<ContextMenu>([](ContextMenu* menu)
                            {
                                //menu->AddItem(L"Test1", []() { MessageBoxA(nullptr, "ContextMenu.Test1", nullptr, 0); });
                                menu->AddItem(L"Test2", []() { MessageBox::Show(L"ContextMenu.Test2", L"ContextMenu.Test2"); });
                            });

                            bubble->OnHotkey(InputModifier::None, InputKey::D, [](ControlBase* self)
                            {
                                self->OpenContextMenu();
                            });

                        });
                    });
                });
            }));

            // ==========================================
            // 2. ПАНЕЛЬ ВВОДА (Строка 1)
            // ==========================================
            AddChild(1, 0, std::make_unique<Border>(init<Grid>([&](Grid* inputGrid)
            {
                inputGrid->HorizontalAlignment = HorizaontalAllign::Stretch;
                inputGrid->AddColumn(ColumnDefinition{ GridLength::Auto() });
                inputGrid->AddColumn(ColumnDefinition{ GridLength::Auto() }); 
                inputGrid->AddColumn(ColumnDefinition{ GridLength::Star(1.0f) });

                inputGrid->AddChild(0, 0, init<Spinner>([&](Spinner* promptSpinner)
                {
                    promptSpinner->Margin = Thickness(1, 0, 1, 0);
                }));

                // Префикс текст бокса
                inputGrid->AddChild(0, 1, init<Label>([&](Label* promptLabel)
                {
                    promptLabel->Text = L"Rikitav@Tamerlan> ";
                    promptLabel->HorizontalAlignment = HorizaontalAllign::Left;
                }));

                // Поле для ввода текста
                inputGrid->AddChild(0, 2, init<TextBox>([&](TextBox* inputBox)
                {
                    inputBox->Text = L"";
                    inputBox->MaxSize = Size(-1, 1);
                    inputBox->HorizontalAlignment = HorizaontalAllign::Stretch;
                    inputBox->AcceptsReturn = false;

                    inputBox->OnHotkey(InputModifier::None, InputKey::RETURN, [&](ControlBase* self)
                    {
                        TextBox* selfTextBox = static_cast<TextBox*>(self);
                        this->chatHistory_.push_back(MessageModel{ true, L"[69:69:69]", selfTextBox->Text });
                        selfTextBox->Text = L"";
                    });
                }));
            })));
            
            // ==========================================
            // 3. СТАТУС-БАР (Строка 2)
            // ==========================================
            AddChild(2, 0, init<Grid>([&](Grid* statusGrid)
            {
                statusGrid->HorizontalAlignment = HorizaontalAllign::Stretch;
                statusGrid->AddColumn(ColumnDefinition{ GridLength::Auto() });
                statusGrid->AddColumn(ColumnDefinition{ GridLength::Star() });

                // Кнопочка от нефиг делать
                statusGrid->AddChild(0, 0, init<Button>([](Button* statusBar)
                {
                    statusBar->Text = L"ESC - выход | /help | Чат: Tamerlan";
                    statusBar->HorizontalAlignment = HorizaontalAllign::Stretch;
                    statusBar->VerticalAlignment = VerticalAlign::Top;

                    statusBar->Clicked += []()
                    {
                        MessageBox::Show(L"Test1", L"ХУУУУУУУУУУУУУУУУУУУУУУУУУУУУУУУУУУУУУУЙ", MessageBoxButton::YesNoCancel);
                    };
                }));

                // Тестовый прогрессбар
                statusGrid->AddChild(0, 1, init<ProgressBar>([](ProgressBar* progress)
                {
                    progress->Margin = Thickness(1, 0, 1, 0);
                    progress->HorizontalAlignment = HorizaontalAllign::Stretch;
                    progress->VerticalAlignment = VerticalAlign::Top;

                    // Онимация
                    DispatchTimer::Current().TickEvent += [progress](float dt)
                    {
                        float current = progress->Value.Get();
                        current += dt * 10.0f; // +10% в секунду

                        if (current > progress->Maximum.Get())
                            current = progress->Minimum.Get();

                        progress->Value = current;
                    };
                }));
            })); 
            
            // ==========================================
            // ГОРЯЧИЕ КЛАВИШИ
            // ==========================================
            OnHotkey(InputModifier::None, InputKey::ESCAPE, [](ControlBase* self)
            {
                HostApplication::Current().RequestStop();
            });

            OnHotkey(InputModifier::None, InputKey::SUBTRACT, [](ControlBase* self)
            {
                auto selectedFile = OpenFileDialog::Show();
                if (!selectedFile.has_value())
                    MessageBox::Show(L"OpenFileDialog test", L"returned nothing", MessageBoxButton::Ok);
                else
                    MessageBox::Show(L"OpenFileDialog test", selectedFile.value().wstring(), MessageBoxButton::Ok);
            });

            OnHotkey(InputModifier::None, InputKey::MULTIPLY, [](ControlBase* self)
            {
                MessageBox::Show(L"Help", L"Help screen", MessageBoxButton::YesNoCancel);
            });
        }
    };
}

int main()
{
	HostApplication& app = HostApplication::Current();
	app.EnterTerminal();
    app.RunUILoop(std::make_unique<TestApp::MessangerTest>());
	app.ExitTerminal();
	return 0;
}
