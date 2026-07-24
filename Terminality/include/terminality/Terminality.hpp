#pragma once
#ifndef _TERMINALITY_

#ifdef _WINDOWS_
  #ifndef TERMINALITY_EMMITED_WINDOWS_ERROR
    #error Include Windows.h after Terminality.hpp
    #define TERMINALITY_EMMITED_WINDOWS_ERROR
  #endif // #ifndef TERMINALITY
#endif // #ifdef _WINDOWS_

// terminality.Core
#include <terminality/Core/Color.hpp>
#include <terminality/Core/Focus.hpp>
#include <terminality/Core/Geometry.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Core/Layout.hpp>
#include <terminality/Core/TextHelper.hpp>

// terminality.Controls.Layout
#include <terminality/Controls/Grid.hpp>
#include <terminality/Controls/StackPanel.hpp>
#include <terminality/Controls/ItemsControl.hpp>
#include <terminality/Controls/ScrollViewer.hpp>
#include <terminality/Controls/TabControl.hpp>
#include <terminality/Controls/TreeView.hpp>
#include <terminality/Controls/Heap.hpp>
#include <terminality/Controls/Expander.hpp>

// terminality.Controls.Visual
#include <terminality/Controls/Label.hpp>
#include <terminality/Controls/Border.hpp>
#include <terminality/Controls/Spinner.hpp>
#include <terminality/Controls/ProgressBar.hpp>
#include <terminality/Controls/DataGrid.hpp>
#include <terminality/Controls/Canvas.hpp>

// terminality.Controls.Interactable
#include <terminality/Controls/CheckBox.hpp>
#include <terminality/Controls/Button.hpp>
#include <terminality/Controls/TextBox.hpp>
#include <terminality/Controls/Slider.hpp>
#include <terminality/Controls/Menu.hpp>
#include <terminality/Controls/MenuBar.hpp>

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

#endif // #ifndef TERMINALITY
