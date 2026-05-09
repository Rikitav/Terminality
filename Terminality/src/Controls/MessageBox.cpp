module terminality;

import std;
import std.compat;

using namespace terminality;

MessageBoxResult MessageBox::Show(const std::wstring& title, const std::wstring& message, MessageBoxButton buttons)
{
    HostApplication& host = HostApplication::Current();
    VisualTree& tree = VisualTree::Current();

    MessageBoxResult result = MessageBoxResult::None;
    std::atomic<bool>* running = nullptr;

    auto rootGrid = init<Grid>([&](Grid* rootGrid)
    {
        rootGrid->HorizontalAlignment = HorizontalAlignment::Stretch;
        rootGrid->VerticalAlignment = VerticalAlignment::Stretch;

        rootGrid->AddChild(0, 0, init<Border>([&](Border* dialogBorder)
        {
            dialogBorder->HorizontalAlignment = HorizontalAlignment::Center;
            dialogBorder->VerticalAlignment = VerticalAlignment::Center;

            if (!title.empty())
            {
                dialogBorder->HeaderText = title;
            }

            dialogBorder->Content = init<StackPanel>([&](StackPanel* dialogContent)
            {
                dialogContent->HorizontalAlignment = HorizontalAlignment::Stretch;
                dialogContent->VerticalAlignment = VerticalAlignment::Top;
                dialogContent->Margin = Thickness(2, 1, 2, 0);

                /*
                if (!title.empty())
                {
                    dialogContent->AddChild(init<Label>([&](Label* titleBox)
                    {
                        titleBox->Text = title;
                        titleBox->SetFocusable(false);
                        titleBox->SetTabStop(false);
                    }));
                }
                */

                dialogContent->AddChild(init<Label>([&](Label* messageBox)
                {
                    messageBox->Text = message;
                    messageBox->SetFocusable(false);
                    messageBox->SetTabStop(false);
                    messageBox->TextWrapping = TextWrapping::WrapWholeWords;
                }));

                dialogContent->AddChild(init<Grid>([&](Grid* buttonGrid)
                {
                    buttonGrid->Margin = Thickness(0, 1, 0, 0);
                    buttonGrid->HorizontalAlignment = HorizontalAlignment::Right;

                    int colIndex = 0;
                    auto addButton = [&](const std::wstring& text, MessageBoxResult res)
                    {
                        buttonGrid->AddColumn(ColumnDefinition{ GridLength::Auto() });
                        buttonGrid->AddChild(0, colIndex++, init<Button>([&](Button* btn)
                        {
                            btn->Text = text;
                            btn->Clicked += [res, &result, &running]()
                            {
                                result = res;
                                running->store(false);
                            };
                        }));
                    };

                    switch (buttons)
                    {
                        case MessageBoxButton::YesNoCancel:
                        {
                            addButton(L"Yes", MessageBoxResult::Yes);
                            addButton(L"No", MessageBoxResult::No);
                            addButton(L"Cancel", MessageBoxResult::Cancel);
                            break;
                        }

                        case MessageBoxButton::YesNo:
                        {
                            addButton(L"Yes", MessageBoxResult::Yes);
                            addButton(L"No", MessageBoxResult::No);
                            break;
                        }

                        case MessageBoxButton::OkCancel:
                        {
                            addButton(L"OK", MessageBoxResult::Ok);
                            addButton(L"Cancel", MessageBoxResult::Cancel);
                            break;
                        }

                        case MessageBoxButton::Ok:
                        {
                            addButton(L"OK", MessageBoxResult::Ok);
                            break;
                        }
                    }
                }));
            });
        }));
    });

    UILayer& layer = tree.PushLayer(std::move(rootGrid));
    running = &layer.Running;

    tree.GetFocusManager().MoveNext(Direction::Next);
    host.NestUILoop(layer);
    tree.PopLayer();
    return result;
}
