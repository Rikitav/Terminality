export module terminality:Label;

import <string>;

import :Geometry;
import :ControlBase;
import :Event;
import :Property;
import :RenderContext;

export namespace terminality
{
	class Label : public ControlBase
	{
		bool isPressed_ = false;

	public:
		Property<Label, std::wstring> Text		 { this, "Text", L"", InvalidationKind::Measure };
		Property<Label, TextWrap> TextWrapping   { this, "TextWrapping", terminality::TextWrap::NoWrap, InvalidationKind::Measure };
		Property<Label, TextAlign> TextAlignment { this, "TextAlignment", terminality::TextAlign::Left, InvalidationKind::Visual };

		Event<> TextChanged;

		Label();
		Label(std::wstring& text);

		void OnPropertyChanged(const char* propertyName) override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}
