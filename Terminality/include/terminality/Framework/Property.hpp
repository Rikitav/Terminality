#pragma once

#include <cstdint>
#include <functional>

#include <terminality/Framework/Event.hpp>

namespace terminality
{
	enum class InvalidationKind
	{
		None = 0,
		Visual = 1,
		Arrange = 2,
		Measure = 4
	};

	template<typename TOwner, typename T>
	class Property
	{
		TOwner* owner_;
		const char* name_;
		T value_;
		InvalidationKind invalidation_;
		std::function<void(const T&)> changing_;

	public:
		Property(TOwner* owner, const char* name, T defaultValue = T(), InvalidationKind invalidation = InvalidationKind::None, std::function<void(const T&)> changing = nullptr);

		Property& operator=(const T& value);
		Property& operator=(T&& value);

		operator const T&() const;
		const T* operator->() const;

		const T& Get() const;
		TOwner& Set(T&& value);

		bool operator==(const T& other) const;
		bool operator!=(const T& other) const;
	};
}

template<typename TOwner, typename T>
terminality::Property<TOwner, T>::Property(TOwner* owner, const char* name, T defaultValue, InvalidationKind invalidation, std::function<void(const T&)> changing)
	: owner_(owner), name_(name), value_(std::move(defaultValue)), invalidation_(invalidation), changing_(std::move(changing)) { }

template<typename TOwner, typename T>
terminality::Property<TOwner, T>& terminality::Property<TOwner, T>::operator=(const T& value)
{
	if (value_ == value)
		return *this;

	if (changing_)
		changing_(value_);

	value_ = value;
	if (owner_)
	{
		owner_->ApplyInvalidation(invalidation_);
		owner_->OnPropertyChanged(name_);
	}

	return *this;
}

template<typename TOwner, typename T>
terminality::Property<TOwner, T>& terminality::Property<TOwner, T>::operator=(T&& value)
{
	if (value_ == value)
		return *this;

	if (changing_)
		changing_(value_);

	value_ = std::move(value);
	if (owner_ != nullptr)
	{
		owner_->ApplyInvalidation(invalidation_);
		owner_->OnPropertyChanged(name_);
	}

	return *this;
}

template<typename TOwner, typename T>
terminality::Property<TOwner, T>::operator const T& () const
{
	return value_;
}

template<typename TOwner, typename T>
const T& terminality::Property<TOwner, T>::Get() const
{
	return value_;
}

template<typename TOwner, typename T>
TOwner& terminality::Property<TOwner, T>::Set(T&& value)
{
	if (value_ == value)
		return *owner_;

	if (changing_)
		changing_(value_);

	value_ = std::move(value);
	if (owner_ != nullptr)
	{
		owner_->ApplyInvalidation(invalidation_);
		owner_->OnPropertyChanged(name_);
	}

	return *owner_;
}

template<typename TOwner, typename T>
const T* terminality::Property<TOwner, T>::operator->() const
{
	return &value_;
}

template<typename TOwner, typename T>
bool terminality::Property<TOwner, T>::operator==(const T& other) const
{
	return value_ == other;
}

template<typename TOwner, typename T>
bool terminality::Property<TOwner, T>::operator!=(const T& other) const
{
	return value_ != other;
}
