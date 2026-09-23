#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include <terminality/Core/Color.hpp>
#include <terminality/Core/Geometry.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Framework/Event.hpp>
#include <terminality/Framework/Property.hpp>
#include <terminality/Framework/Collections/ObservableCollection.hpp>
#include <terminality/Engine/RenderContext.hpp>

namespace terminality
{
	/// Barrel-style selection list. The currently selected item renders in the
	/// vertical center of the control; the previous and next items render directly
	/// above and below it. UP/DOWN arrow keys spin the barrel, moving the selection
	/// backward/forward through Items.
	class BarrelListBox : public ControlBase
	{
		int32_t lastSelectedIndex_ = 0;

		EventConnection<std::size_t, const std::wstring&> itemAdded_;
		EventConnection<std::size_t, const std::wstring&> itemRemoved_;
		EventConnection<std::size_t, const std::wstring&, const std::wstring&> itemReplaced_;
		EventConnection<> collectionCleared_;

		void OnItemAdded(std::size_t index, const std::wstring& item);
		void OnItemRemoved(std::size_t index, const std::wstring& item);
		void OnItemReplaced(std::size_t index, const std::wstring& oldItem, const std::wstring& newItem);
		void OnCollectionCleared();

		void EnsureSelectionValid();

	public:
		ObservableCollection<std::wstring> Items;

		// Fired with the new index whenever the selection changes.
		Event<std::size_t> SelectionChanged;

		Property<BarrelListBox, int32_t> SelectedIndex { this, "SelectedIndex", 0, InvalidationKind::Visual };
		Property<BarrelListBox, int32_t> VisibleRows   { this, "VisibleRows",   5, InvalidationKind::Measure };
		Property<BarrelListBox, bool>    Wrap          { this, "Wrap",          true, InvalidationKind::Visual };

		BarrelListBox();

		const std::wstring* GetSelectedItem() const;

		void OnPropertyChanged(const char* propertyName) override;

		bool IsFocusable() const override { return true; }
		bool OnKeyDown(InputEvent input) override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}
