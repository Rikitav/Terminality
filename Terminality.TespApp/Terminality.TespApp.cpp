
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <functional>

//#include <terminality/Terminality.hpp>
#define TERMINALITY_IMPLEMENTATION
#include "../out/Terminality.hpp"

#include <Windows.h>
#undef MessageBox

using namespace terminality;

// --- Helper Functions to create tests ---

void ShowModal(const std::wstring& title, std::unique_ptr<ControlBase> content)
{
    auto container = init<Grid>([&](Grid* grid)
    {
        grid->HorizontalAlignment = HorizontalAlign::Stretch;
        grid->VerticalAlignment = VerticalAlign::Stretch;
        grid->AddRow(RowDefinition{GridLength::Star()});
        
        grid->AddChild(0, 0, init<Border>([&](Border* b)
        {
            b->HeaderText = title + L" (ESC to close)";
            b->Content = std::move(content);
            
            b->OnHotkey(InputModifier::None, InputKey::ESCAPE, [](ControlBase* self)
            {
                self->Close();
            });
        }));
    });

    UILayer& layer = VisualTree::Current().PushLayer(std::move(container));
    HostApplication::Current().NestUILoop(layer);
    VisualTree::Current().PopLayer();
}

struct TestModalDef
{
    std::wstring Name;
    std::wstring Description;
    std::function<std::unique_ptr<ControlBase>()> Factory;
};

// --- Widget Tests ---

std::unique_ptr<ControlBase> TestButton()
{
    return init<StackPanel>([](StackPanel* p)
    {
        p->ContentOrientation = Orientation::Vertical;
        p->HorizontalContentAlignment = HorizontalAlign::Center;
        p->VerticalContentAlignment = VerticalAlign::Center;
        
        p->AddChild(init<Button>([](Button* b)
        {
            b->Text = L"Click Me!";
            b->Clicked += []()
            {
                MessageBox::Show(L"Button Test", L"Button was clicked!", MessageBoxButton::Ok);
            };
        }));
    });
}

std::unique_ptr<ControlBase> TestCheckBox()
{
    return init<StackPanel>([](StackPanel* p)
    {
        p->ContentOrientation = Orientation::Vertical;
        p->HorizontalContentAlignment = HorizontalAlign::Center;
        p->VerticalContentAlignment = VerticalAlign::Center;
        
        p->AddChild(init<CheckBox>([](CheckBox* cb) { cb->Text = L"Option 1 (Unchecked)"; }));
        p->AddChild(init<CheckBox>([](CheckBox* cb) { cb->Text = L"Option 2 (Checked)"; cb->Toggle(true); }));
        p->AddChild(init<CheckBox>([](CheckBox* cb) { cb->Text = L"Option 3 (Disabled focus, theoretically)"; cb->SetFocusable(false); }));
    });
}

std::unique_ptr<ControlBase> TestTextBox()
{
    return init<StackPanel>([](StackPanel* p)
    {
        p->ContentOrientation = Orientation::Vertical;
        p->HorizontalContentAlignment = HorizontalAlign::Center;
        p->VerticalContentAlignment = VerticalAlign::Center;
        
        p->AddChild(init<Label>([](Label* l)
        {
            l->Text = L"Enter some text below:";
        }));

        p->AddChild(init<TextBox>([](TextBox* tb)
        {
            tb->Text = L"Default text";
            tb->MinSize = Size(20, 1);
        }));

        p->AddChild(init<TextBox>([](TextBox* tb)
        {
            tb->Text = L"Multi-line (AcceptsReturn = true)\nLine2";
            tb->AcceptsReturn = true;
            tb->MinSize = Size(30, 3);
        }));
    });
}

std::unique_ptr<ControlBase> TestLabelAndBorder()
{
    return init<Grid>([](Grid* g)
    {
        g->HorizontalAlignment = HorizontalAlign::Stretch;
        g->VerticalAlignment = VerticalAlign::Stretch;
        g->AddRow(RowDefinition{GridLength::Auto()});
        g->AddRow(RowDefinition{GridLength::Auto()});
        
        g->AddChild(0, 0, init<Label>([](Label* l)
        {
            l->Text = L"This is a standard label.\nIt can have multiple lines.";
            l->Margin = Thickness(0, 1, 0, 1);
        }));
        
        g->AddChild(1, 0, init<Border>([](Border* b)
        {
            b->HeaderText = L"Border Widget";
            b->Content = init<Label>([](Label* l)
            {
                l->Text = L"Content inside a border";
            });
        }));
    });
}

std::unique_ptr<ControlBase> TestProgressBarAndSpinner()
{
    return init<StackPanel>([](StackPanel* p)
    {
        p->ContentOrientation = Orientation::Vertical;
        p->HorizontalContentAlignment = HorizontalAlign::Stretch;
        
        p->AddChild(init<Label>([](Label* l)
        {
            l->Text = L"Animated Spinner:";
        }));

        p->AddChild(init<Spinner>([](Spinner* s)
        {
            s->Margin = Thickness(0, 1, 0, 1);
        }));
        
        p->AddChild(init<Label>([](Label* l)
        {
            l->Text = L"Animated ProgressBar:";
        }));

        p->AddChild(init<ProgressBar>([](ProgressBar* pb)
        {
            pb->Margin = Thickness(0, 1, 0, 1);
            DispatchTimer::Current().TickEvent += [pb](float dt)
            {
                float v = pb->Value.Get() + dt * 15.0f;
                if (v > pb->Maximum.Get()) v = pb->Minimum.Get();
                pb->Value = v;
            };
        }));
    });
}

// --- Container Tests ---

std::unique_ptr<ControlBase> TestGrid1()
{
    return init<Grid>([](Grid* g)
    {
        g->AddRow(RowDefinition{GridLength::Cell(3)});
        g->AddRow(RowDefinition{GridLength::Star()});
        g->AddColumn(ColumnDefinition{GridLength::Cell(10)});
        g->AddColumn(ColumnDefinition{GridLength::Star()});
        
        g->AddChild(0, 0, init<Border>([](Border* b) { b->Content = init<Label>([](Label* l){ l->Text=L"R0, C0";}); }));
        g->AddChild(0, 1, init<Border>([](Border* b) { b->Content = init<Label>([](Label* l){ l->Text=L"R0, C1 (Star)";}); }));
        g->AddChild(1, 0, init<Border>([](Border* b) { b->Content = init<Label>([](Label* l){ l->Text=L"R1, C0 (Star)";}); }));
        g->AddChild(1, 1, init<Border>([](Border* b) { b->Content = init<Label>([](Label* l){ l->Text=L"R1, C1 (StarxStar)";}); }));
    });
}

std::unique_ptr<ControlBase> TestGrid2()
{
    return init<Grid>([](Grid* g)
    {
        g->AddRow(RowDefinition{GridLength::Star()});
        g->AddRow(RowDefinition{GridLength::Star()});
        g->AddColumn(ColumnDefinition{GridLength::Star()});
        g->AddColumn(ColumnDefinition{GridLength::Star()});
        
        // ColSpan test
        g->AddChild(0, 0, 1, 2, init<Border>([](Border* b) { b->HeaderText = L"ColSpan 2"; b->Content = init<Label>([](Label* l){ l->Text=L"Spans two columns";}); }));
        g->AddChild(1, 0, init<Border>([](Border* b) { b->Content = init<Label>([](Label* l){ l->Text=L"R1, C0";}); }));
        g->AddChild(1, 1, init<Border>([](Border* b) { b->Content = init<Label>([](Label* l){ l->Text=L"R1, C1";}); }));
    });
}

std::unique_ptr<ControlBase> TestGrid3()
{
    return init<Grid>([](Grid* g)
    {
        g->AddRow(RowDefinition{GridLength::Star()});
        g->AddRow(RowDefinition{GridLength::Star()});
        g->AddColumn(ColumnDefinition{GridLength::Star()});
        g->AddColumn(ColumnDefinition{GridLength::Star()});
        
        // RowSpan test
        g->AddChild(0, 0, 2, 1, init<Border>([](Border* b) { b->HeaderText = L"RowSpan 2"; b->Content = init<Label>([](Label* l){ l->Text=L"Spans two rows";}); }));
        g->AddChild(0, 1, init<Border>([](Border* b) { b->Content = init<Label>([](Label* l){ l->Text=L"R0, C1";}); }));
        g->AddChild(1, 1, init<Border>([](Border* b) { b->Content = init<Label>([](Label* l){ l->Text=L"R1, C1";}); }));
    });
}

std::unique_ptr<ControlBase> TestStackPanel1()
{
    return init<StackPanel>([](StackPanel* p)
    {
        p->ContentOrientation = Orientation::Vertical;
        p->AddChild(init<Button>([](Button* b) { b->Text = L"Btn 1"; }));
        p->AddChild(init<Button>([](Button* b) { b->Text = L"Btn 2"; }));
        p->AddChild(init<Button>([](Button* b) { b->Text = L"Btn 3"; }));
    });
}

std::unique_ptr<ControlBase> TestStackPanel2()
{
    return init<StackPanel>([](StackPanel* p)
    {
        p->ContentOrientation = Orientation::Horizontal;
        p->AddChild(init<Button>([](Button* b) { b->Text = L"Left"; }));
        p->AddChild(init<Button>([](Button* b) { b->Text = L"Middle"; }));
        p->AddChild(init<Button>([](Button* b) { b->Text = L"Right"; }));
    });
}

std::unique_ptr<ControlBase> TestStackPanel3()
{
    return init<StackPanel>([](StackPanel* p)
    {
        p->ContentOrientation = Orientation::Vertical;
        p->HorizontalContentAlignment = HorizontalAlign::Center;
        p->VerticalContentAlignment = VerticalAlign::Center;
        p->AddChild(init<Label>([](Label* l){ l->Text = L"Centered Vertical StackPanel"; }));
        p->AddChild(init<Button>([](Button* b){ b->Text = L"Action"; }));
    });
}

std::unique_ptr<ControlBase> TestItemsControl1()
{
    static ObservableCollection<std::wstring> items;
    if (items.size() == 0)
    {
        items.push_back(L"Item A");
        items.push_back(L"Item B");
        items.push_back(L"Item C");
    }
    
    return init<ItemsControl<std::wstring>>([](ItemsControl<std::wstring>* ic)
    {
        ic->SetItemsSource(&items);
        ic->SetItemTemplate([](const std::wstring& item)
        {
            return init<Button>([&](Button* b)
            {
                b->Text = item;
            });
        });
    });
}

std::unique_ptr<ControlBase> TestItemsControl2()
{
    static ObservableCollection<std::wstring> items2;
    if (items2.size() == 0)
    {
        for(int i = 0; i < 20; ++i)
            items2.push_back(L"Horizontal " + std::to_wstring(i));
    }
    
    return init<ItemsControl<std::wstring>>([](ItemsControl<std::wstring>* ic)
    {
        ic->ContentOrientation = Orientation::Horizontal;
        ic->Scrollable = true;
        ic->SetItemsSource(&items2);
        ic->SetItemTemplate([](const std::wstring& item)
        {
            return init<Border>([&](Border* b)
            { 
                b->Content = init<Label>([&](Label* l) { l->Text = item; }); 
            });
        });
    });
}

std::unique_ptr<ControlBase> TestItemsControl3()
{
    static ObservableCollection<int> numbers;
    if (numbers.size() == 0)
    {
        for(int i = 0; i < 50; ++i)
            numbers.push_back(i);
    }
    
    return init<ItemsControl<int>>([](ItemsControl<int>* ic)
    {
        ic->ContentOrientation = Orientation::Vertical;
        ic->Scrollable = true;
        ic->AutoScrollToEnd = true;
        ic->SetItemsSource(&numbers);
        ic->SetItemTemplate([](const int& item)
        {
            return init<Label>([&](Label* l) { l->Text = L"Number: " + std::to_wstring(item); });
        });
    });
}

std::unique_ptr<ControlBase> TestScrollViewer1()
{
    return init<ScrollViewer>([](ScrollViewer* sv)
    {
        sv->Content = init<StackPanel>([](StackPanel* p)
        {
            for(int i = 0; i < 30; ++i)
            {
                p->AddChild(init<Label>([i](Label* l)
                {
                    l->Text = L"Line " + std::to_wstring(i);
                }));
            }
        });
    });
}

std::unique_ptr<ControlBase> TestScrollViewer2()
{
    return init<ScrollViewer>([](ScrollViewer* sv)
    {
        sv->Content = init<Grid>([](Grid* g)
        {
            g->AddRow(RowDefinition{GridLength::Cell(50)});
            g->AddColumn(ColumnDefinition{GridLength::Cell(50)});
            g->AddChild(0, 0, init<Button>([](Button* b){ b->Text = L"Huge Button"; }));
        });
    });
}

std::unique_ptr<ControlBase> TestScrollViewer3()
{
    return init<ScrollViewer>([](ScrollViewer* sv)
    {
        sv->Content = init<StackPanel>([](StackPanel* p)
        {
            p->ContentOrientation = Orientation::Horizontal;
            for(int i = 0; i < 20; ++i)
            {
                p->AddChild(init<Button>([i](Button* b){ b->Text = L"Btn" + std::to_wstring(i); }));
            }
        });
    });
}

std::unique_ptr<ControlBase> TestTabControl1()
{
    return init<TabControl>([](TabControl* tc)
    {
        tc->AddTab("Tab 1", init<Label>([](Label* l) { l->Text = L"Content of Tab 1"; }));
        tc->AddTab("Tab 2", init<Button>([](Button* b) { b->Text = L"Button in Tab 2"; }));
        tc->AddTab("Tab 3", init<TextBox>([](TextBox* t) { t->Text = L"Text in Tab 3"; }));
    });
}

std::unique_ptr<ControlBase> TestTabControl2()
{
    return init<TabControl>([](TabControl* tc)
    {
        for(int i=0; i<10; ++i)
        {
            tc->AddTab("Tab " + std::to_string(i), init<Label>([i](Label* l)
            {
                l->Text = L"Content " + std::to_wstring(i);
            }));
        }
    });
}

std::unique_ptr<ControlBase> TestTabControl3()
{
    return init<TabControl>([](TabControl* tc)
    {
        tc->AddTab("Controls", init<StackPanel>([](StackPanel* p)
        {
            p->AddChild(init<CheckBox>([](CheckBox* cb){ cb->Text = L"Check!"; }));
            p->AddChild(init<Button>([](Button* b){ b->Text = L"Click!"; }));
        }));
    
        tc->AddTab("Empty", nullptr);
        
        tc->AddTab("Grid", init<Grid>([](Grid* g)
        {
            g->AddRow(RowDefinition{GridLength::Star()});
            g->AddColumn(ColumnDefinition{GridLength::Star()});
            g->AddChild(0,0, init<Label>([](Label* l){ l->Text = L"Inside grid inside tab"; }));
        }));
    });
}

// --- Visuals and Hotkeys ---

std::unique_ptr<ControlBase> TestVisuals()
{
    return init<StackPanel>([](StackPanel* p)
    {
        p->ContentOrientation = Orientation::Vertical;
        p->AddChild(init<Label>([](Label* l)
        {
            l->Text = L"Custom Colors!";
            l->ForegroundColor = Color::CYAN;
            l->BackgroundColor = Color::DARK_BLUE;
        }));
        
        p->AddChild(init<Button>([](Button* b)
        {
            b->Text = L"Focused Colors!";
            b->FocusedForegroundColor = Color::YELLOW;
            b->FocusedBackgroundColor = Color::DARK_RED;
        }));
    });
}

std::unique_ptr<ControlBase> TestHotkeys()
{
    return init<StackPanel>([](StackPanel* p)
    {
        p->ContentOrientation = Orientation::Vertical;
        p->AddChild(init<Label>([](Label* l)
        {
            l->Text = L"Press 'H' or 'CTRL+H' for greetings!";
            
            l->OnHotkey(InputModifier::None, InputKey::H, [](ControlBase*)
            {
                MessageBox::Show(L"Hotkey", L"'H' pressed!", MessageBoxButton::Ok);
            });
            
            l->OnHotkey(InputModifier::LeftCtrl, InputKey::H, [](ControlBase*)
            {
                MessageBox::Show(L"Hotkey", L"'CTRL+H' pressed!", MessageBoxButton::Ok);
            });
        }));
    });
}

// --- Main App ---

class TestingAppMainMenu : public Grid
{
    ObservableCollection<TestModalDef> tests_;

public:
    TestingAppMainMenu()
    {
        DispatchTimer::Current().SetUIThread();
        HorizontalAlignment = HorizontalAlign::Stretch;
        VerticalAlignment = VerticalAlign::Stretch;
        
        AddRow(RowDefinition{GridLength::Auto()});
        AddRow(RowDefinition{GridLength::Star()});
        
        AddChild(0, 0, init<Border>([](Border* b)
        {
            b->Content = init<Label>([](Label* l)
            {
                l->Text = L"Terminality Framework Test Suite. Select a test using UP/DOWN, press ENTER.";
                l->HorizontalAlignment = HorizontalAlign::Center;
            });
        }));

        tests_.push_back({L"Button Test", L"Basic Button widget", TestButton});
        tests_.push_back({L"CheckBox Test", L"Check box variations", TestCheckBox});
        tests_.push_back({L"TextBox Test", L"Input variations", TestTextBox});
        tests_.push_back({L"Label & Border Test", L"Static visuals", TestLabelAndBorder});
        tests_.push_back({L"ProgressBar & Spinner", L"Animated widgets", TestProgressBarAndSpinner});
        
        tests_.push_back({L"Grid Test 1", L"Star and Cell lengths", TestGrid1});
        tests_.push_back({L"Grid Test 2", L"Column Spans", TestGrid2});
        tests_.push_back({L"Grid Test 3", L"Row Spans", TestGrid3});
        
        tests_.push_back({L"StackPanel Test 1", L"Vertical stack", TestStackPanel1});
        tests_.push_back({L"StackPanel Test 2", L"Horizontal stack", TestStackPanel2});
        tests_.push_back({L"StackPanel Test 3", L"Alignment stack", TestStackPanel3});
        
        tests_.push_back({L"ItemsControl Test 1", L"Basic Items", TestItemsControl1});
        tests_.push_back({L"ItemsControl Test 2", L"Horizontal Items", TestItemsControl2});
        tests_.push_back({L"ItemsControl Test 3", L"Auto-Scroll Items", TestItemsControl3});
        
        tests_.push_back({L"ScrollViewer Test 1", L"Vertical scrolling", TestScrollViewer1});
        tests_.push_back({L"ScrollViewer Test 2", L"2D scrolling", TestScrollViewer2});
        tests_.push_back({L"ScrollViewer Test 3", L"Horizontal scrolling", TestScrollViewer3});
        
        tests_.push_back({L"TabControl Test 1", L"Basic tabs", TestTabControl1});
        tests_.push_back({L"TabControl Test 2", L"Truncation/Many tabs", TestTabControl2});
        tests_.push_back({L"TabControl Test 3", L"Nested layouts in tabs", TestTabControl3});
        
        tests_.push_back({L"Visuals Test", L"Custom colors", TestVisuals});
        tests_.push_back({L"Hotkeys Test", L"Input handling", TestHotkeys});
        
        AddChild(1, 0, init<ItemsControl<TestModalDef>>([&](ItemsControl<TestModalDef>* ic)
        {
            ic->SetItemsSource(&tests_);
            ic->Scrollable = true;
            ic->SetItemTemplate([](const TestModalDef& item)
            {
                return init<Button>([item](Button* b)
                {
                    b->Text = item.Name + L" - " + item.Description;
                    b->HorizontalAlignment = HorizontalAlign::Stretch;
                    b->Clicked += [item]()
                    {
                        ShowModal(item.Name, item.Factory());
                    };
                });
            });
        }));
        
        OnHotkey(InputModifier::None, InputKey::ESCAPE, [](ControlBase*)
        {
            HostApplication::Current().RequestStop();
        });
    }
};

int main()
{
    try
    {
        HostApplication& app = HostApplication::Current();
        app.EnterTerminal();
        app.RunUILoop(std::make_unique<TestingAppMainMenu>());
        app.ExitTerminal();
        return 0;
    }
	catch (const std::exception& ex)
	{
		std::cerr << "Unhandled exception: " << ex.what() << std::endl;
		return -1;
	}
    catch (...)
    {
        return -1;
    }
}
