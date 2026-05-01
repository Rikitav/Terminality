module terminality;

import std;

using namespace terminality;

template<typename T>
inline PropertyDescriptor<T>::PropertyDescriptor(const T& defaultValue) : defaultValue_(defaultValue)
{
	//static_assert(std::equality_comparable<T>, "Type must support operator==");
}

template<typename T>
const T& PropertyDescriptor<T>::GetDefaultValue() const
{
	return defaultValue_;
}

template<typename T>
inline bool DependencyObject::SetLocalValue(const PropertyDescriptor<T>& key, const T& value)
{
	const auto found = values_.find(key.Id);
	if (found != values_.end())
	{
		const T* oldValue = std::any_cast<T>(&found->second);
		if (oldValue && *oldValue == value)
		{
			return false;
		}

		found->second = value;
		return true;
	}

	values_[key.Id] = value;
	return true;
}

template<typename T>
inline std::optional<T> DependencyObject::TryGetLocalValue(const PropertyDescriptor<T>& key) const
{
	const auto found = values_.find(key.Id);
	if (found == values_.end())
	{
		return std::nullopt;
	}

	const T* typed = std::any_cast<T>(&found->second);
	if (!typed)
	{
		throw std::logic_error(std::string("Property type mismatch for key: ") + key.Name);
	}

	return *typed;
}

template<typename T>
inline void DependencyObject::SetValue(const PropertyDescriptor<T>& key, const T& value)
{
	if (!SetLocalValue(key, value))
		return;

	ApplyInvalidation(key.Invalidation);
	PropertyChanged.Emit(key.Name);
}

template<typename T>
inline T DependencyObject::GetValue(const PropertyDescriptor<T>& key) const
{
	const auto localValue = TryGetLocalValue(key);
	if (localValue.has_value())
		return localValue.value();

	if (!key.Inheritable)
		return key.DefaultValue;

	const DependencyObject* parentUi = static_cast<const DependencyObject*>(GetParent());
	if (parentUi == nullptr)
		return key.DefaultValue;

	return parentUi->GetValue(key);
}

void DependencyObject::ApplyInvalidation(InvalidationKind invalidation)
{
	switch (invalidation)
	{
		case InvalidationKind::Visual:
		{
			InvalidateVisual();
			break;
		}

		case InvalidationKind::Arrange:
		{
			InvalidateArrange();
			break;
		}

		case InvalidationKind::Measure:
		{
			InvalidateMeasure();
			break;
		}
	}
}

void DependencyObject::OnPropertyChanged(const char* propertyName)
{
	return;
}
