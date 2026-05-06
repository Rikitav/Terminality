export module terminality:ItemsControl;

import std;
import :ControlBase;
import :StackPanel;
import :ObservableCollection;
import :EventSignal;

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

		std::optional<EventSignalConnection<size_t, const T&>> addedConnection_;
		std::optional<EventSignalConnection<size_t, const T&>> removedConnection_;
		std::optional<EventSignalConnection<size_t, const T&, const T&>> replacedConnection_;
		std::optional<EventSignalConnection<>> clearedConnection_;

		void RebuildItems()
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

		void OnItemAdded(size_t index, const T& item)
		{
			if (itemTemplate_)
			{
				this->Insert(index, itemTemplate_(item));
			}
		}

		void OnItemRemoved(size_t index, const T& item)
		{
			this->RemoveAt(index);
		}

		void OnItemReplaced(size_t index, const T& oldItem, const T& newItem)
		{
			if (itemTemplate_)
			{
				this->RemoveAt(index);
				this->Insert(index, itemTemplate_(newItem));
			}
		}

		void OnCollectionCleared()
		{
			this->Clear();
		}

	public:
		ItemsControl() = default;
		virtual ~ItemsControl() = default;

		void SetItemTemplate(ItemTemplate itemTemplate)
		{
			itemTemplate_ = std::move(itemTemplate);
			RebuildItems();
		}

		void SetItemsSource(ObservableCollection<T>* itemsSource)
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
					[this](size_t index, const T& item) { OnItemAdded(index, item); }));
				
				removedConnection_.emplace(itemsSource_->ItemRemoved.Connect(
					[this](size_t index, const T& item) { OnItemRemoved(index, item); }));
				
				replacedConnection_.emplace(itemsSource_->ItemReplaced.Connect(
					[this](size_t index, const T& oldItem, const T& newItem) { OnItemReplaced(index, oldItem, newItem); }));
				
				clearedConnection_.emplace(itemsSource_->CollectionCleared.Connect(
					[this]() { OnCollectionCleared(); }));
			}

			RebuildItems();
		}

		ObservableCollection<T>* GetItemsSource() const
		{
			return itemsSource_;
		}
	};
}