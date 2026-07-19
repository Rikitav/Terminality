// terminality.Core
#include <terminality/Core/Color.hpp>
#include <terminality/Core/Focus.hpp>
#include <terminality/Core/Geometry.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Core/Layout.hpp>
#include <terminality/Core/TextHelper.hpp>

// terminality.Controls.Layout
#include <terminality/Controls/Layout/Grid.hpp>
#include <terminality/Controls/Layout/StackPanel.hpp>
#include <terminality/Controls/Layout/ItemsControl.hpp>
#include <terminality/Controls/Layout/ScrollViewer.hpp>
#include <terminality/Controls/Layout/TabControl.hpp>
#include <terminality/Controls/Layout/TreeView.hpp>
#include <terminality/Controls/Layout/Heap.hpp>
#include <terminality/Controls/Layout/Expander.hpp>

// terminality.Controls.Visual
#include <terminality/Controls/Visual/Label.hpp>
#include <terminality/Controls/Visual/Border.hpp>
#include <terminality/Controls/Visual/Spinner.hpp>
#include <terminality/Controls/Visual/ProgressBar.hpp>
#include <terminality/Controls/Visual/DataGrid.hpp>
#include <terminality/Controls/Visual/Canvas.hpp>

// terminality.Controls.Interactable
#include <terminality/Controls/Interactable/CheckBox.hpp>
#include <terminality/Controls/Interactable/Button.hpp>
#include <terminality/Controls/Interactable/TextBox.hpp>
#include <terminality/Controls/Interactable/Slider.hpp>

// terminality.Dialogs
#include <terminality/Dialogs/ContextMenu.hpp>
#include <terminality/Dialogs/MessageBox.hpp>
#include <terminality/Dialogs/OpenFileDialog.hpp>

// terminality.Engine
#include <terminality/Engine/DispatchTimer.hpp>
#include <terminality/Engine/FocusManager.hpp>
#include <terminality/Engine/Navigator.hpp>
#include <terminality/Engine/RenderBuffer.hpp>
#include <terminality/Engine/RenderContext.hpp>
#include <terminality/Engine/RenderStream.hpp>

// terminality.Framework.Collections
#include <terminality/Framework/Collections/ObservableCollection.hpp>
#include <terminality/Framework/Collections/Queries.hpp>

// terminality.Framework
#include <terminality/Framework/VisualTree.hpp>
#include <terminality/Framework/VisualTreeNode.hpp>
#include <terminality/Framework/Property.hpp>
#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Framework/Event.hpp>
#include <terminality/Framework/HostApplication.hpp>

// terminality.PlatformSpecific
#ifdef _WIN32
	#include <terminality/Framework/PlatformSpecific/Windows.hpp>
#endif // _WIN32