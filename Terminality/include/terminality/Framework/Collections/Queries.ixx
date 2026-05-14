export module terminality:Queries;

import <cstdint>;
import <vector>;
import <type_traits>;
import <ranges>;
import <algorithm>;
import <stdexcept>;

export namespace terminality::queries
{
	template <typename Derived>
	struct QueryOperator {};

	template <std::ranges::range R, typename Derived>
	auto operator|(R&& range, const QueryOperator<Derived>& op)
	{
		return static_cast<const Derived&>(op)(std::forward<R>(range));
	}

	// Where (Filter)
	template <typename Predicate>
	struct WhereOp : QueryOperator<WhereOp<Predicate>>
	{
		Predicate pred;
		template <std::ranges::range R>
		auto operator()(R&& range) const
		{
			return std::forward<R>(range) | std::views::filter(pred);
		}
	};

	template <typename Predicate>
	WhereOp<std::decay_t<Predicate>> Where(Predicate&& pred)
	{
		return { std::forward<Predicate>(pred) };
	}

	// Select (Transform)
	template <typename Selector>
	struct SelectOp : QueryOperator<SelectOp<Selector>>
	{
		Selector sel;
		template <std::ranges::range R>
		auto operator()(R&& range) const
		{
			return std::forward<R>(range) | std::views::transform(sel);
		}
	};

	template <typename Selector>
	SelectOp<std::decay_t<Selector>> Select(Selector&& sel)
	{
		return { std::forward<Selector>(sel) };
	}

	// ToList
	struct ToListOp : QueryOperator<ToListOp>
	{
		template <std::ranges::range R>
		auto operator()(R&& range) const
		{
			using ElementType = std::ranges::range_value_t<R>;
			std::vector<ElementType> result;
			if constexpr (std::ranges::sized_range<R>)
				result.reserve(std::ranges::size(range));

			for (auto&& item : range)
				result.push_back(std::forward<decltype(item)>(item));

			return result;
		}
	};

	inline ToListOp ToList()
	{
		return {};
	}

	// First
	template <typename Predicate>
	struct FirstPredOp : QueryOperator<FirstPredOp<Predicate>>
	{
		Predicate pred;
		template <std::ranges::range R>
		auto operator()(R&& range) const
		{
			for (auto&& item : range)
			{
				if (pred(item))
					return item;
			}

			throw std::runtime_error("Sequence contains no matching element");
		}
	};

	template <typename Predicate>
	FirstPredOp<std::decay_t<Predicate>> First(Predicate&& pred)
	{
		return { std::forward<Predicate>(pred) };
	}

	struct FirstOp : QueryOperator<FirstOp>
	{
		template <std::ranges::range R>
		auto operator()(R&& range) const
		{
			auto it = std::ranges::begin(range);
			if (it != std::ranges::end(range))
				return *it;
			
			throw std::runtime_error("Sequence contains no elements");
		}
	};

	inline FirstOp First()
	{
		return {};
	}

	// FirstOrDefault
	template <typename Predicate>
	struct FirstOrDefaultPredOp : QueryOperator<FirstOrDefaultPredOp<Predicate>>
	{
		Predicate pred;
		template <std::ranges::range R>
		auto operator()(R&& range) const
		{
			using ElementType = std::ranges::range_value_t<R>;
			for (auto&& item : range)
			{
				if (pred(item))
					return std::optional<ElementType>(item);
			}
			
			return std::optional<ElementType>(std::nullopt);
		}
	};

	template <typename Predicate>
	FirstOrDefaultPredOp<std::decay_t<Predicate>> FirstOrDefault(Predicate&& pred)
	{
		return { std::forward<Predicate>(pred) };
	}

	struct FirstOrDefaultOp : QueryOperator<FirstOrDefaultOp>
	{
		template <std::ranges::range R>
		auto operator()(R&& range) const
		{
			using ElementType = std::ranges::range_value_t<R>;
			auto it = std::ranges::begin(range);
			if (it != std::ranges::end(range))
				return std::optional<ElementType>(*it);
			
			return std::optional<ElementType>(std::nullopt);
		}
	};

	inline FirstOrDefaultOp FirstOrDefault()
	{
		return {};
	}

	// Any
	template <typename Predicate>
	struct AnyPredOp : QueryOperator<AnyPredOp<Predicate>>
	{
		Predicate pred;
		template <std::ranges::range R>
		bool operator()(R&& range) const
		{
			for (auto&& item : range)
			{
				if (pred(item))
					return true;
			}
			
			return false;
		}
	};

	template <typename Predicate>
	AnyPredOp<std::decay_t<Predicate>> Any(Predicate&& pred)
	{
		return { std::forward<Predicate>(pred) };
	}

	struct AnyOp : QueryOperator<AnyOp>
	{
		template <std::ranges::range R>
		bool operator()(R&& range) const
		{
			return !std::ranges::empty(range);
		}
	};

	inline AnyOp Any()
	{
		return {};
	}

	// All
	template <typename Predicate>
	struct AllOp : QueryOperator<AllOp<Predicate>>
	{
		Predicate pred;
		template <std::ranges::range R>
		bool operator()(R&& range) const
		{
			for (auto&& item : range)
			{
				if (!pred(item))
					return false;
			}
			
			return true;
		}
	};

	template <typename Predicate>
	AllOp<std::decay_t<Predicate>> All(Predicate&& pred)
	{
		return { std::forward<Predicate>(pred) };
	}

	// Count
	struct CountOp : QueryOperator<CountOp>
	{
		template <std::ranges::range R>
		auto operator()(R&& range) const
		{
			return std::ranges::distance(range);
		}
	};

	inline CountOp Count()
	{
		return {};
	}

	template <typename Predicate>
	struct CountPredOp : QueryOperator<CountPredOp<Predicate>>
	{
		Predicate pred;
		template <std::ranges::range R>
		auto operator()(R&& range) const
		{
			std::size_t c = 0;
			for (auto&& item : range)
			{
				if (pred(item))
					c++;
			}
			
			return c;
		}
	};

	template <typename Predicate>
	CountPredOp<std::decay_t<Predicate>> Count(Predicate&& pred)
	{
		return { std::forward<Predicate>(pred) };
	}

	// OrderBy
	template <typename KeySelector>
	struct OrderByOp : QueryOperator<OrderByOp<KeySelector>>
	{
		KeySelector sel;
		template <std::ranges::range R>
		auto operator()(R&& range) const
		{
			using ElementType = std::ranges::range_value_t<R>;
			std::vector<ElementType> result;
			if constexpr (std::ranges::sized_range<R>)
			{
				result.reserve(std::ranges::size(range));
			}
			
			for (auto&& item : range)
			{
				result.push_back(item);
			}
			
			std::ranges::sort(result, {}, sel);
			return result;
		}
	};

	template <typename KeySelector>
	OrderByOp<std::decay_t<KeySelector>> OrderBy(KeySelector&& sel)
	{
		return { std::forward<KeySelector>(sel) };
	}

	// OrderByDescending
	template <typename KeySelector>
	struct OrderByDescendingOp : QueryOperator<OrderByDescendingOp<KeySelector>>
	{
		KeySelector sel;
		template <std::ranges::range R>
		auto operator()(R&& range) const
		{
			using ElementType = std::ranges::range_value_t<R>;
			std::vector<ElementType> result;
			
			if constexpr (std::ranges::sized_range<R>)
			{
				result.reserve(std::ranges::size(range));
			}
			
			for (auto&& item : range)
			{
				result.push_back(item);
			}
			
			std::ranges::sort(result, std::greater<>{}, sel);
			return result;
		}
	};

	template <typename KeySelector>
	OrderByDescendingOp<std::decay_t<KeySelector>> OrderByDescending(KeySelector&& sel)
	{
		return { std::forward<KeySelector>(sel) };
	}
}