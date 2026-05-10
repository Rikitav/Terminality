module terminality;

import std;
import std.compat;

using namespace terminality;

MessageBoxResult MessageBox::Show(const std::wstring& title, const std::wstring& message, MessageBoxButton buttons)
{
    MessageBoxResult result = MessageBoxResult::None;
    
    auto rootGrid = init<Grid>([&](Grid* rootGrid)
    {
        rootGrid->HorizontalAlignment = HorizaontalAllign::Stretch;
        rootGrid->VerticalAlignment = VerticalAlign::Stretch;

        rootGrid->AddChild(0, 0, init<Border>([&](Border* dialogBorder)
        {
            dialogBorder->HorizontalAlignment = HorizaontalAllign::Center;
            dialogBorder->VerticalAlignment = VerticalAlign::Center;

            if (!title.empty())
            {
                dialogBorder->HeaderText = title;
            }

            dialogBorder->Content = init<StackPanel>([&](StackPanel* dialogContent)
            {
                dialogContent->HorizontalAlignment = HorizaontalAllign::Stretch;
                dialogContent->VerticalAlignment = VerticalAlign::Top;
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
                    messageBox->TextWrapping = TextWrap::WrapWholeWords;
                }));

                dialogContent->AddChild(init<Grid>([&](Grid* buttonGrid)
                {
                    buttonGrid->Margin = Thickness(0, 1, 0, 0);
                    buttonGrid->HorizontalAlignment = HorizaontalAllign::Right;

                    int colIndex = 0;
                    auto addButton = [&](const std::wstring& text, MessageBoxResult res)
                    {
                        buttonGrid->AddColumn(ColumnDefinition{ GridLength::Auto() });
                        buttonGrid->AddChild(0, colIndex++, init<Button>([&](Button* btn)
                        {
                            btn->Text = text;
                            btn->Clicked += [btn, res, &result]()
                            {
                                result = res;
                                btn->Close();
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

    Navigator::Current().Navigate(std::move(rootGrid));
    return result;
}
