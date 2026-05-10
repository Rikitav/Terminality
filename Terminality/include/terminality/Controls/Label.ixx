export module terminality:Label;

import std;
import :Geometry;
import :ControlBase;
import :EventSignal;
import :PropertyDescriptor;
import :RenderContext;

export namespace terminality
{
	class Label : public ControlBase
	{
		bool isPressed_ = false;

	public:
		PropertyDescriptor<Label, std::wstring> Text		   { this, "Text", L"", InvalidationKind::Measure };
		PropertyDescriptor<Label, TextWrap> TextWrapping   { this, "TextWrapping", terminality::TextWrap::NoWrap, InvalidationKind::Measure };
		PropertyDescriptor<Label, TextAlign> TextAlignment { this, "TextAlignment", terminality::TextAlign::Left, InvalidationKind::Visual };

		EventSignal<> TextChanged;

		Label();
		Label(std::wstring& text);

		void OnPropertyChanged(const char* propertyName) override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}

