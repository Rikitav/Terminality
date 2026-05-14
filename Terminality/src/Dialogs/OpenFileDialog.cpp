module terminality;

import std;
import std.compat;

using namespace terminality;

struct FileSystemEntry
{
    std::wstring Name;
    bool IsDirectory;
    bool IsParentDir;
};

class FileSystemEntryView : public Button
{
public:
    FileSystemEntry Model;

    FileSystemEntryView()
    {
        HorizontalAlignment = HorizontalAlign::Stretch;
    }

    Size MeasureOverride(const Size& availableSize) override
    {
        // Выделяем место под "[ " + имя + " ]"
        return Size(static_cast<int>(Model.Name.size()) + 4, 1);
    }

    // Переопределяем ТОЛЬКО отрисовку.
    // Вся логика OnKeyDown, OnMouseDown и Clicked.Emit() 
    // бесплатно достается нам от родительского Button!
    void RenderOverride(RenderContext& context) override
    {
        auto rin = context.BeginText();

        // Используем состояние focused_, которое менеджит Button/ControlBase
        if (focused_)
        {
            rin << SetBack(Color::WHITE) << SetFore(Color::BLACK);
        }
        else
        {
            rin << SetBack(Color::BLACK) << SetFore(Color::WHITE);
        }

        rin << L"[ ";
        if (!focused_)
        {
            if (Model.IsParentDir)
                rin << SetFore(Color::YELLOW);
            else if (Model.IsDirectory)
                rin << SetFore(Color::CYAN);
            else
                rin << SetFore(Color::LIGHT_GRAY);
        }

        rin << Model.Name;

        if (!focused_)
            rin << SetFore(Color::WHITE);

        rin << L" ]";
    }
};

std::optional<std::filesystem::path> OpenFileDialog::Show(const std::wstring& title, const std::filesystem::path& initialDireinity)
{
    // WIP
    //*
    HostApplication& host = HostApplication::Current();
    VisualTree& tree = VisualTree::Current();

    std::optional<std::filesystem::path> result = std::nullopt;
	std::atomic<bool>* running = nullptr;

    std::filesystem::path currentDir = initialDireinity.empty()
        ? std::filesystem::current_path()
        : initialDireinity;

    ObservableCollection<FileSystemEntry> currentFiles;
    TextBox* pathBoxPtr = nullptr;

    auto loadDirectory = [&]()
    {
        currentFiles.clear();

        if (pathBoxPtr != nullptr)
            pathBoxPtr->Text = currentDir.wstring();

        // Добавляем ".."
        if (currentDir.has_parent_path() && currentDir != currentDir.parent_path())
        {
            currentFiles.push_back(FileSystemEntry{ L"..", true, true });
        }

        try
        {
            std::vector<FileSystemEntry> dirs;
            std::vector<FileSystemEntry> files;

            for (const auto& entry : std::filesystem::directory_iterator(currentDir))
            {
                if (entry.is_directory())
                    dirs.push_back(FileSystemEntry{ entry.path().filename().wstring(), true, false });
                else
                    files.push_back(FileSystemEntry{ entry.path().filename().wstring(), false, false });
            }

            auto sortAlphabetically = [](const FileSystemEntry& a, const FileSystemEntry& b)
            {
                return a.Name < b.Name;
            };

            std::sort(dirs.begin(), dirs.end(), sortAlphabetically);
            std::sort(files.begin(), files.end(), sortAlphabetically);

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
        root->HorizontalAlignment = HorizontalAlign::Stretch;
        root->VerticalAlignment = VerticalAlign::Stretch;

        root->AddChild(0, 0, init<Border>([&](Border* dialogBorder)
        {
            dialogBorder->HorizontalAlignment = HorizontalAlign::Center;
            dialogBorder->VerticalAlignment = VerticalAlign::Center;
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
                    pathBox->HorizontalAlignment = HorizontalAlign::Stretch;
                }));

                dialogContent->AddChild(1, 0, init<ItemsControl<FileSystemEntry>>([&](ItemsControl<FileSystemEntry>* items)
                {
                    items->HorizontalAlignment = HorizontalAlign::Stretch;
                    items->VerticalAlignment = VerticalAlign::Stretch;
                    items->SetItemsSource(&currentFiles);

                    items->SetItemTemplate([&](const FileSystemEntry& entryModel) -> std::unique_ptr<ControlBase>
                    {
                        return init<FileSystemEntryView>([&, entryModel](FileSystemEntryView* view)
                        {
                            view->Model = entryModel;
                            view->HorizontalAlignment = HorizontalAlign::Stretch;

                            view->Clicked += [&currentDir, &result, &running, &loadDirectory, entryModel]()
                            {
                                if (entryModel.IsParentDir)
                                {
                                    currentDir = currentDir.parent_path();
                                    loadDirectory();
                                }
                                else if (entryModel.IsDirectory)
                                {
                                    currentDir /= entryModel.Name;
                                    loadDirectory();
                                }
                                else
                                {
                                    result = currentDir / entryModel.Name;
                                    running->store(false);
                                }
                            };
                        });
                    });
                }));

                dialogContent->AddChild(2, 0, init<Button>([&](Button* cancelBtn)
                {
                    cancelBtn->Text = L"Cancel";
                    cancelBtn->HorizontalAlignment = HorizontalAlign::Right;
                    cancelBtn->Clicked += [&running]()
                    {
                        running->store(false);
                    };
                }));
            });
        }));
    });

    loadDirectory();
    UILayer& layer = tree.PushLayer(std::move(rootGrid));
    running = &layer.Running;

    host.NestUILoop(layer);
    tree.PopLayer();
    return result;
    //*/

    return std::nullopt;
}
