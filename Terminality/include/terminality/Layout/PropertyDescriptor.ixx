export module terminality:PropertyDescriptor;

import std;
import :EventSignal;

export namespace terminality
{
	enum class InvalidationKind
	{
		None = 0,
		Visual = 1,
		Arrange = 2,
		Measure = 4
	};

	template<typename TOwner, typename T>
	class PropertyDescriptor
	{
		TOwner* owner_;
		const char* name_;
		T value_;
		InvalidationKind invalidation_;

	public:
		/*
		PropertyDescriptor(TOwner* owner, const char* name, const T& defaultValue = T(), InvalidationKind invalidation = InvalidationKind::None)
			: owner_(owner), name_(name), value_(defaultValue), invalidation_(invalidation) { }
		*/
		
		PropertyDescriptor(TOwner* owner, const char* name, T defaultValue = T(), InvalidationKind invalidation = InvalidationKind::None)
			: owner_(owner), name_(name), value_(std::move(defaultValue)), invalidation_(invalidation) {}

		PropertyDescriptor& operator=(const T& value)
		{
			if (value_ == value)
				return *this;

			value_ = value;
			if (owner_)
			{
				owner_->ApplyInvalidation(invalidation_);
				owner_->OnPropertyChanged(name_);
			}

			return *this;
		}
		
		PropertyDescriptor& operator=(T&& value)
		{
			if (value_ == value)
				return *this;

			value_ = std::move(value);
			if (owner_ != nullptr)
			{
				owner_->ApplyInvalidation(invalidation_);
				owner_->OnPropertyChanged(name_);
			}

			return *this;
		}

		operator const T&() const
		{
			return value_;
		}

		const T& Get() const
		{
			return value_;
		}
		
		TOwner& Set(T&& value)
		{
			if (value_ == value)
				return *owner_;

			value_ = std::move(value);
			if (owner_ != nullptr)
			{
				owner_->ApplyInvalidation(invalidation_);
				owner_->OnPropertyChanged(name_);
			}

			return *owner_;
		}

		const T* operator->() const
		{
			return &value_;
		}

		bool operator==(const T& other) const
		{
			return value_ == other;
		}

		bool operator!=(const T& other) const
		{
			return value_ != other;
		}
	};
}