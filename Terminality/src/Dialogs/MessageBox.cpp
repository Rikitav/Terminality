
#include <string>
#include <memory>
#include <functional>

#include <terminality/Dialogs/MessageBox.hpp>
#include <terminality/Framework/HostApplication.hpp>
#include <terminality/Engine/Navigator.hpp>

#include <terminality/Controls/Grid.hpp>
#include <terminality/Controls/Border.hpp>
#include <terminality/Controls/StackPanel.hpp>
#include <terminality/Controls/Label.hpp>
#include <terminality/Controls/Button.hpp>

// The Windows headers define MessageBox as a macro; this translation unit's
// code (and everything after it in the single-header amalgamation) needs the
// class name. The class declaration itself is protected by the same #undef in
// MessageBox.hpp, which runs before <Windows.h> is included there.
#ifdef MessageBox
#undef MessageBox
#endif

using namespace terminality;

MessageBoxResult MessageBox::Show(const std::wstring& title, const std::wstring& message, MessageBoxButton buttons)
{
    MessageBoxResult result = MessageBoxResult::None;
    
    auto rootGrid = init<Grid>([&](Grid* root)
    {
        root->HorizontalAlignment = HorizontalAlign::Stretch;
        root->VerticalAlignment = VerticalAlign::Stretch;

        root->AddChildControl(0, 0, init<Border>([&](Border* dialogBorder)
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

                dialogContent->AddChildControl(init<Label>([&](Label* messageBox)
                {
                    messageBox->Text = message;
                    messageBox->SetFocusable(false);
                    messageBox->SetTabStop(false);
                    messageBox->TextWrapping = TextWrap::WrapWholeWords;
                }));

                dialogContent->AddChildControl(init<Grid>([&](Grid* buttonGrid)
                {
                    buttonGrid->Margin = Thickness(0, 1, 0, 0);
                    buttonGrid->HorizontalAlignment = HorizontalAlign::Right;

                    int colIndex = 0;
                    auto addButton = [&](const std::wstring& text, MessageBoxResult res)
                    {
                        buttonGrid->AddColumn(ColumnDefinition{ GridLength::Auto() });
                        buttonGrid->AddChildControl(0, colIndex++, init<Button>([&](Button* btn)
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
