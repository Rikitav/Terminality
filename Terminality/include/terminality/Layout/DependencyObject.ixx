export module terminality:DependencyObject;

import std;
import :EventSignal;
import :UIElement;

export namespace terminality
{
	enum class InvalidationKind
	{
		None,
		Visual,
		Arrange,
		Measure
	};

	template<typename T>
	class PropertyDescriptor
	{
		const char* name_;
		T defaultValue_;
		InvalidationKind invalidation_;

	public:
		PropertyDescriptor(const T& defaultValue);
		const T& GetDefaultValue() const;
	};

	class DependencyObject : public UIElement
	{
		std::unordered_map<size_t, std::any> values_;

		template<typename T> bool SetLocalValue(const PropertyDescriptor<T>& key, const T& value);
		template<typename T> std::optional<T> TryGetLocalValue(const PropertyDescriptor<T>& key) const;

	public:
		EventSignal<const char*> PropertyChanged;

		template<typename T> void SetValue(const PropertyDescriptor<T>& key, const T& value);
		template<typename T> T GetValue(const PropertyDescriptor<T>& key) const;

		void ApplyInvalidation(InvalidationKind invalidation);
		virtual void OnPropertyChanged(const char* propertyName);
	};
}