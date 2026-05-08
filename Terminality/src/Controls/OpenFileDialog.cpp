module terminality;

import std;
import std.compat;

using namespace terminality;

std::optional<std::filesystem::path> OpenFileDialog::Show(const std::wstring& title, const std::filesystem::path& initialDireinity)
{
    // WIP
    /*
    HostApplication& host = HostApplication::Current();
    VisualTree& tree = VisualTree::Current();

    std::optional<std::filesystem::path> result = std::nullopt;
    std::atomic<bool> running = true;

    std::filesystem::path currentDir = initialDireinity.empty()
        ? std::filesystem::current_path()
        : initialDireinity;

    ObservableCollection<std::wstring> currentFiles;
    TextBox* pathBoxPtr = nullptr;

    auto loadDirectory = [&]()
    {
        currentFiles.clear();

        if (pathBoxPtr != nullptr)
            pathBoxPtr->Text = currentDir.wstring();

        if (currentDir.has_parent_path() && currentDir != currentDir.parent_path())
        {
            currentFiles.push_back(L"..");
        }

        try
        {
            std::vector<std::wstring> dirs;
            std::vector<std::wstring> files;

            for (const auto& entry : std::filesystem::directory_iterator(currentDir))
            {
                if (entry.is_directory())
                    dirs.push_back(L"[" + entry.path().filename().wstring() + L"]");
                else
                    files.push_back(entry.path().filename().wstring());
            }

            std::sort(dirs.begin(), dirs.end());
            std::sort(files.begin(), files.end());

            for (const auto& d : dirs)
                currentFiles.push_back(d);

            for (const auto& f : files)
                currentFiles.push_back(f);
        }
        catch (...)
        {

        }
    };
    
    auto rootGrid = init<Grid>([&](Grid* root)
    {
        root->HorizontalAlignment = HorizontalAlignment::Stretch;
        root->VerticalAlignment = VerticalAlignment::Stretch;

        root->AddChild(0, 0, init<Border>([&](Border* dialogBorder)
        {
            dialogBorder->HorizontalAlignment = HorizontalAlignment::Center;
            dialogBorder->VerticalAlignment = VerticalAlignment::Center;
            dialogBorder->BorderColor = Color::WHITE;
            dialogBorder->MaxSize = Size(60, 20);
            dialogBorder->MinSize = Size(40, 10);

            dialogBorder->Content = init<Grid>([&](Grid* dialogContent)
            {
                dialogContent->AddRow(RowDefinition{ GridLength::Auto() });
                dialogContent->AddRow(RowDefinition{ GridLength::Star(1.0f) });
                dialogContent->AddRow(RowDefinition{ GridLength::Auto() });

                dialogContent->AddChild(0, 0, init<TextBox>([&](TextBox* pathBox)
                {
                    pathBoxPtr = pathBox;
                    pathBox->Text = currentDir.wstring();
                    pathBox->SetFocusable(false);
                    pathBox->SetTabStop(false);
                    pathBox->HorizontalAlignment = HorizontalAlignment::Stretch;
                }));

                dialogContent->AddChild(1, 0, init<ItemsControl<std::wstring>>([&](ItemsControl<std::wstring>* items)
                {
                    items->HorizontalAlignment = HorizontalAlignment::Stretch;
                    items->VerticalAlignment = VerticalAlignment::Stretch;
                    items->SetItemsSource(&currentFiles);

                    items->SetItemTemplate([&](const std::wstring& item) -> std::unique_ptr<ControlBase>
                    {
                        return init<Button>([&](Button* btn)
                        {
                            btn->Text = item;
                            btn->HorizontalAlignment = HorizontalAlignment::Stretch;
                            btn->Clicked += [&currentDir, &result, &running, &loadDirectory, item]()
                            {
                                if (item == L"..")
                                {
                                    currentDir = currentDir.parent_path();
                                    loadDirectory();
                                }
                                else if (item.starts_with(L"[") && item.ends_with(L"]"))
                                {
                                    std::wstring dirName = item.substr(1, item.length() - 2);
                                    currentDir /= dirName;
                                    loadDirectory();
                                }
                                else
                                {
                                    result = currentDir / item;
                                    running.store(false);
                                }
                            };
                        });
                    });
                }));

                dialogContent->AddChild(2, 0, init<Button>([&](Button* cancelBtn)
                {
                    cancelBtn->Text = L"Cancel";
                    cancelBtn->HorizontalAlignment = HorizontalAlignment::Right;
                    cancelBtn->Clicked += [&running]()
                    {
                        running.store(false);
                    };
                }));
            });
        }));
    });

    loadDirectory();

    tree.PushLayer(std::move(rootGrid));
    host.RunUILoop(running);
    tree.PopLayer();

    return result;
    */

    return std::nullopt;
}
