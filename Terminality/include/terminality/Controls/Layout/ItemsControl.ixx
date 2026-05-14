export module terminality:ItemsControl;

import <cstdint>;

import :ControlBase;
import :StackPanel;
import :ObservableCollection;
import :FocusManager;
import :InputEvent;
import :Focus;
import :Event;

export namespace terminality
{
	template<typename T>
	class ItemsControl : public StackPanel
	{
	public:
		using ItemTemplate = std::function<std::unique_ptr<ControlBase>(const T&)>;

	private:
		ObservableCollection<T>* itemsSource_ = nullptr;
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

	template<typename T>
	bool ItemsControl<T>::MoveFocusNext(Direction direction, InputModifier modifiers)
	{
		if (modifiers == InputModifier::Special)
		{
			focusedIndex_ = 0;
			return true;
		}

		return StackPanel::MoveFocusNext(direction, modifiers);
	}

	template<typename T>
	void ItemsControl<T>::RebuildItems()
	{
		this->Clear();
		if (itemsSource_ && itemTemplate_)
		{
			for (const auto& item : *itemsSource_)
			{
				this->AddChild(itemTemplate_(item));
			}
		}
	}

	template<typename T>
	void ItemsControl<T>::OnItemAdded(std::size_t index, const T& item)
	{
		if (itemTemplate_)
		{
			this->Insert(index, itemTemplate_(item));
		}
	}

	template<typename T>
	void ItemsControl<T>::OnItemRemoved(std::size_t index, const T& item)
	{
		this->RemoveAt(index);
	}

	template<typename T>
	void ItemsControl<T>::OnItemReplaced(std::size_t index, const T& oldItem, const T& newItem)
	{
		if (itemTemplate_)
		{
			this->RemoveAt(index);
			this->Insert(index, itemTemplate_(newItem));
		}
	}

	template<typename T>
	void ItemsControl<T>::OnCollectionCleared()
	{
		FocusManager::Current().MoveNext(Direction::Previous, InputModifier::Special);
		this->Clear();
	}

	template<typename T>
	void ItemsControl<T>::SetItemTemplate(ItemTemplate itemTemplate)
	{
		itemTemplate_ = std::move(itemTemplate);
		RebuildItems();
	}

	template<typename T>
	void ItemsControl<T>::SetItemsSource(ObservableCollection<T>* itemsSource)
	{
		if (itemsSource_ == itemsSource)
			return;

		if (itemsSource_)
		{
			addedConnection_.reset();
			removedConnection_.reset();
			replacedConnection_.reset();
			clearedConnection_.reset();
		}

		itemsSource_ = itemsSource;

		if (itemsSource_)
		{
			addedConnection_.emplace(itemsSource_->ItemAdded.Connect(
				[this](std::size_t index, const T& item) { OnItemAdded(index, item); }));

			removedConnection_.emplace(itemsSource_->ItemRemoved.Connect(
				[this](std::size_t index, const T& item) { OnItemRemoved(index, item); }));

			replacedConnection_.emplace(itemsSource_->ItemReplaced.Connect(
				[this](std::size_t index, const T& oldItem, const T& newItem) { OnItemReplaced(index, oldItem, newItem); }));

			clearedConnection_.emplace(itemsSource_->CollectionCleared.Connect(
				[this]() { OnCollectionCleared(); }));
		}

		RebuildItems();
	}

	template<typename T>
	ObservableCollection<T>* ItemsControl<T>::GetItemsSource() const
	{
		return itemsSource_;
	}
}
