import std;
import std.compat;
import terminality;

using namespace terminality;

namespace
{
    struct MessageModel
    {
        bool isAuthor;
        std::wstring Timestamp;
        std::wstring Text;
    };

    class MessageBubble : public ControlBase
    {
    public:
        MessageModel message_;

        MessageBubble()
        {

        }

        Size MeasureOverride(const Size& availableSize) override
        {
            static const int timeStampLength = 10;
            return Size(timeStampLength + message_.Text.size() + 1, 1);
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
                rin << SetFore(Color::BLACK) << message_.Timestamp;
            }
            else
            {
                rin << SetBack(Color::BLACK);
                rin << SetFore(Color::WHITE) << message_.Timestamp;
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
            HorizontalAlignment = HorizontalAlignment::Stretch;
            VerticalAlignment = VerticalAlignment::Stretch;

            AddRow(RowDefinition{ GridLength::Star(1.0f) });   // Строка 0: Основной чат
            AddRow(RowDefinition{ GridLength::Pixel(3) });     // Строка 1: Поле ввода
            AddRow(RowDefinition{ GridLength::Pixel(2) });     // Строка 2: Футер (подсказки)

            // ==========================================
            // 1. ИСТОРИЯ ЧАТА (Строка 0)
            // ==========================================
            chatHistory_.push_back(MessageModel { true, L"[04:12:04]", L"123" });
            chatHistory_.push_back(MessageModel { false, L"[04:13:42]", L"Привет!" });
            chatHistory_.push_back(MessageModel { true, L"[04:13:51]", L"Tamerlan: Hello, World!" });

            AddChild(0, 0, ctor<Border>([&](Border* chatBorder)
            {
                chatBorder->HeaderText = L"Чат: Tamerlan";
                chatBorder->Content = ctor<ItemsControl<MessageModel>>([&](ItemsControl<MessageModel>*chatList)
                {
                    chatList->SetItemsSource(&chatHistory_);
                    chatList->SetItemTemplate([](const MessageModel& item) -> std::unique_ptr<ControlBase>
                    {
                        return ctor<MessageBubble>([&](MessageBubble* msg)
                        {
                            msg->message_ = item;
                        });
                    });
                });
            }));

            // ==========================================
            // 2. ПАНЕЛЬ ВВОДА (Строка 1)
            // ==========================================
            AddChild(1, 0, std::make_unique<Border>(ctor<Grid>([&](Grid* inputGrid)
            {
                inputGrid->HorizontalAlignment = HorizontalAlignment::Stretch;
                inputGrid->AddColumn(ColumnDefinition{ GridLength::Auto() }); 
                inputGrid->AddColumn(ColumnDefinition{ GridLength::Star(1.0f) });

                // Префикс текст бокса
                inputGrid->AddChild(0, 0, ctor<Label>([&](Label* promptLabel)
                {
                    promptLabel->Text = L"Rikitav@Tamerlan> ";
                    promptLabel->HorizontalAlignment = HorizontalAlignment::Left;
                }));

                // Поле для ввода текста
                inputGrid->AddChild(0, 1, ctor<TextBox>([&](TextBox* inputBox)
                {
                    inputBox->Text = L"";
                    inputBox->MaxSize = Size(-1, 1);
                    inputBox->HorizontalAlignment = HorizontalAlignment::Stretch;
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
            AddChild(2, 0, ctor<Button>([&](Button* statusBar)
            {
                statusBar->Text = L"ESC - выход | /help | Чат: Tamerlan";
                statusBar->HorizontalAlignment = HorizontalAlignment::Stretch;
            }));

            // ==========================================
            // ГОРЯЧИЕ КЛАВИШИ
            // ==========================================
            OnHotkey(InputModifier::None, InputKey::ESCAPE, [](ControlBase* self)
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
    //FocusManager::Current().MoveNext(Direction::Next);

    app.RunUILoop();
	app.ExitTerminal();
	return 0;
}
