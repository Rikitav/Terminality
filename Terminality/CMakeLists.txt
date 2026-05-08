add_library(terminality STATIC)
target_include_directories(terminality PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_sources(terminality
    PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
        include/terminality/Collections/ObservableCollection.ixx
        include/terminality/Collections/Queries.ixx
        include/terminality/Controls/Border.ixx
        include/terminality/Controls/Button.ixx
        include/terminality/Controls/CheckBox.ixx
        include/terminality/Controls/ContextMenu.ixx
        include/terminality/Controls/Grid.ixx
        include/terminality/Controls/ItemsControl.ixx
        include/terminality/Controls/MessageBox.ixx
        include/terminality/Controls/OpenFileDialog.ixx
        include/terminality/Controls/StackPanel.ixx
        include/terminality/Controls/TextBox.ixx
        include/terminality/Controls/ZStack.ixx
        include/terminality/EventSignal.ixx
        include/terminality/FocusManager.ixx
        include/terminality/HostApplication.ixx
        include/terminality/Layout/ControlBase.ixx
        include/terminality/Layout/DependencyObject.ixx
        include/terminality/Layout/UIElement.ixx
        include/terminality/Layout/VisualTree.ixx
        include/terminality/Layout/VisualTreeNode.ixx
        include/terminality/Models/Focus.ixx
        include/terminality/Models/Geometry.ixx
        include/terminality/Models/InputEvent.ixx
        include/terminality/Models/Layout.ixx
        include/terminality/RenderBuffer.ixx
        include/terminality/RenderContext.ixx
        include/terminality/Terminality.ixx
    PRIVATE
        src/Controls/Border.cpp
        src/Controls/Button.cpp
        src/Controls/CheckBox.cpp
        src/Controls/ContextMenu.cpp
        src/Controls/Grid.cpp
        src/Controls/MessageBox.cpp
        src/Controls/OpenFileDialog.cpp
        src/Controls/StackPanel.cpp
        src/Controls/TextBox.cpp
        src/Controls/ZStack.cpp
        src/EventSignal.cpp
        src/FocusManager.cpp
        src/HostApplication.cpp
        src/Layout/ControlBase.cpp
        src/Layout/DependencyObject.cpp
        src/Layout/UIElement.cpp
        src/Layout/VisualTree.cpp
        src/Layout/VisualTreeNode.cpp
        src/Models/Geometry.cpp
        src/RenderBuffer.cpp
        src/RenderContext.cpp
)

if (WIN32)
  target_sources(terminality PRIVATE
        src/PlatformSpecific/HostApplication.Windows.cpp
        src/PlatformSpecific/HostBackend.Windows.cpp
    )
elseif (UNIX)
  target_sources(terminality PRIVATE
        src/PlatformSpecific/HostApplication.Linux.cpp
        src/PlatformSpecific/HostBackend.Linux.cpp
    )
endif()
