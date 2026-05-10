export module terminality:ObservableCollection;

import std;
import :EventSignal;

export namespace terminality
{
	template<typename T>
	class ObservableCollection
	{
		std::vector<T> items_;

	public:
		Event<std::size_t, const T&> ItemAdded;
		Event<std::size_t, const T&> ItemRemoved;
		Event<std::size_t, const T&, const T&> ItemReplaced;
		Event<> CollectionCleared;

		ObservableCollection() = default;
		~ObservableCollection() = default;

		// Iterators
		auto begin() { return items_.begin(); }
		auto end() { return items_.end(); }
		auto begin() const { return items_.begin(); }
		auto end() const { return items_.end(); }
		auto cbegin() const { return items_.cbegin(); }
		auto cend() const { return items_.cend(); }

		// Capacity
		std::size_t size() const { return items_.size(); }
		bool empty() const { return items_.empty(); }

		// Element access
		T& operator[](std::size_t index) { return items_[index]; }
		const T& operator[](std::size_t index) const { return items_[index]; }

		T& at(std::size_t index) { return items_.at(index); }
		const T& at(std::size_t index) const { return items_.at(index); }

		// Modifiers
		void push_back(const T& item)
		{
			items_.push_back(item);
			ItemAdded.Emit(items_.size() - 1, item);
		}

		void push_back(T&& item)
		{
			items_.push_back(std::move(item));
			ItemAdded.Emit(items_.size() - 1, items_.back());
		}

		void pop_back()
		{
			if (items_.empty()) return;
			T removedItem = std::move(items_.back());
			std::size_t index = items_.size() - 1;
			items_.pop_back();
			ItemRemoved.Emit(index, removedItem);
		}

		void insert(std::size_t index, const T& item)
		{
			if (index > items_.size())

				throw std::out_of_range("Index out of range");
			items_.insert(items_.begin() + index, item);
			ItemAdded.Emit(index, item);
		}

		void insert(std::size_t index, T&& item)
		{
			if (index > items_.size())
				throw std::out_of_range("Index out of range");

			items_.insert(items_.begin() + index, std::move(item));
			ItemAdded.Emit(index, items_[index]);
		}

		void erase(std::size_t index)
		{
			if (index >= items_.size())
				throw std::out_of_range("Index out of range");

			T removedItem = std::move(items_[index]);
			items_.erase(items_.begin() + index);
			ItemRemoved.Emit(index, removedItem);
		}

		void replace(std::size_t index, const T& item)
		{
			if (index >= items_.size())
				throw std::out_of_range("Index out of range");

			T oldItem = std::move(items_[index]);
			items_[index] = item;
			ItemReplaced.Emit(index, oldItem, item);
		}

		void replace(std::size_t index, T&& item)
		{
			if (index >= items_.size())
				throw std::out_of_range("Index out of range");

			T oldItem = std::move(items_[index]);
			items_[index] = std::move(item);
			ItemReplaced.Emit(index, oldItem, items_[index]);
		}

		void clear()
		{
			if (items_.empty())
				return;

			items_.clear();
			CollectionCleared.Emit();
		}
	};
}