
#include <string>
#include <memory>
#include <functional>

#include <terminality/Terminality.hpp>

using namespace terminality;

MessageBoxResult MessageBox::Show(const std::wstring& title, const std::wstring& message, MessageBoxButton buttons)
{
    MessageBoxResult result = MessageBoxResult::None;
    
    auto rootGrid = init<Grid>([&](Grid* root)
    {
        root->HorizontalAlignment = HorizontalAlign::Stretch;
        root->VerticalAlignment = VerticalAlign::Stretch;

        root->AddChild(0, 0, init<Border>([&](Border* dialogBorder)
        {
            dialogBorder->HorizontalAlignment = HorizontalAlign::Center;
            dialogBorder->VerticalAlignment = VerticalAlign::Center;

            if (!title.empty())
            {
                dialogBorder->HeaderText = title;
            }

            dialogBorder->Content = init<StackPanel>([&](StackPanel* dialogContent)
            {
                dialogContent->HorizontalAlignment = HorizontalAlign::Stretch;
                dialogContent->VerticalAlignment = VerticalAlign::Top;
                dialogContent->Margin = Thickness(2, 1, 2, 0);

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
                    buttonGrid->HorizontalAlignment = HorizontalAlign::Right;

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
