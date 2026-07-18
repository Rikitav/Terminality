#pragma once

#include <cstdint>
#include <optional>
#include <functional>

#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Controls/Layout/StackPanel.hpp>
#include <terminality/Framework/Collections/ObservableCollection.hpp>
#include <terminality/Engine/FocusManager.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Core/Focus.hpp>
#include <terminality/Framework/Event.hpp>

namespace terminality
{
	template<typename T>
	class ItemsControl : public StackPanel
	{
	public:
		using ItemTemplate = std::function<std::unique_ptr<ControlBase>(const T&)>;

	private:
		std::optional<std::reference_wrapper<ObservableCollection<T>>> itemsSource_;
		ItemTemplate itemTemplate_;

		std::optional<EventConnection<std::size_t, const T&>> addedConnection_;
		std::optional<EventConnection<std::size_t, const T&>> removedConnection_;
		std::optional<EventConnection<std::size_t, const T&, const T&>> replacedConnection_;
		std::optional<EventConnection<>> clearedConnection_;

		void RebuildItems();
		void OnItemAdded(std::size_t index, const T& item);
		void OnItemRemoved(std::size_t index, const T& item);
		void OnItemReplaced(std::size_t index, const T& oldItem, const T& newItem);
		void OnCollectionCleared();

	public:
		ItemsControl() = default;
		virtual ~ItemsControl() = default;

		bool MoveFocusNext(Direction direction, InputModifier modifiers) override;

		void SetItemTemplate(ItemTemplate itemTemplate);
		void SetItemsSource(ObservableCollection<T>* itemsSource);

		ObservableCollection<T>* GetItemsSource() const;
	};
}

template<typename T>
bool terminality::ItemsControl<T>::MoveFocusNext(Direction direction, InputModifier modifiers)
{
	return StackPanel::MoveFocusNext(direction, modifiers);
}

template<typename T>
void terminality::ItemsControl<T>::RebuildItems()
{
	this->Clear();
	if (itemsSource_ && itemTemplate_)
	{
		for (const auto& item : itemsSource_->get())
		{
			this->AddChild(itemTemplate_(item));
		}
	}
}

template<typename T>
void terminality::ItemsControl<T>::OnItemAdded(std::size_t index, const T& item)
{
	if (itemTemplate_)
	{
		this->Insert(index, itemTemplate_(item));
	}
}

template<typename T>
void terminality::ItemsControl<T>::OnItemRemoved(std::size_t index, const T& item)
{
	this->RemoveAt(index);
}

template<typename T>
void terminality::ItemsControl<T>::OnItemReplaced(std::size_t index, const T& oldItem, const T& newItem)
{
	if (itemTemplate_)
	{
		this->RemoveAt(index);
		this->Insert(index, itemTemplate_(newItem));
	}
}

template<typename T>
void terminality::ItemsControl<T>::OnCollectionCleared()
{
	FocusManager::Current().MoveNext(Direction::Previous, InputModifier::Special);
	this->Clear();
}

template<typename T>
void terminality::ItemsControl<T>::SetItemTemplate(ItemTemplate itemTemplate)
{
	itemTemplate_ = std::move(itemTemplate);
	RebuildItems();
}

template<typename T>
void terminality::ItemsControl<T>::SetItemsSource(terminality::ObservableCollection<T>* itemsSource)
{
	bool sameSource = (itemsSource == nullptr && !itemsSource_) ||
		(itemsSource_ && &itemsSource_->get() == itemsSource);
	if (sameSource)
		return;

	addedConnection_.reset();
	removedConnection_.reset();
	replacedConnection_.reset();
	clearedConnection_.reset();
	itemsSource_.reset();

	if (itemsSource != nullptr)
	{
		itemsSource_.emplace(*itemsSource);

		addedConnection_.emplace(itemsSource_->get().ItemAdded.Connect(
			[this](std::size_t index, const T& item) { OnItemAdded(index, item); }));

		removedConnection_.emplace(itemsSource_->get().ItemRemoved.Connect(
			[this](std::size_t index, const T& item) { OnItemRemoved(index, item); }));

		replacedConnection_.emplace(itemsSource_->get().ItemReplaced.Connect(
			[this](std::size_t index, const T& oldItem, const T& newItem) { OnItemReplaced(index, oldItem, newItem); }));

		clearedConnection_.emplace(itemsSource_->get().CollectionCleared.Connect(
			[this]() { OnCollectionCleared(); }));
	}

	RebuildItems();
}

template<typename T>
terminality::ObservableCollection<T>* terminality::ItemsControl<T>::GetItemsSource() const
{
	return itemsSource_ ? &itemsSource_->get() : nullptr;
}
