export module terminality:TextBox;

import std;
import :Geometry;
import :ControlBase;
import :EventSignal;
import :RenderContext;
import :InputEvent;
import :Layout;
import :Focus;

export namespace terminality
{
	class TextBox : public ControlBase
	{
		size_t cursorPosition_ = 0;

	public:
		PropertyDescriptor<TextBox, std::wstring> Text			 { this, "Text", L"", InvalidationKind::Measure };
		PropertyDescriptor<TextBox, TextWrapping> TextWrapping	 { this, "TextWrapping", terminality::TextWrapping::NoWrap, InvalidationKind::Measure };
		PropertyDescriptor<TextBox, TextAlignment> TextAlignment { this, "TextAlignment", terminality::TextAlignment::Left, InvalidationKind::Visual };
		PropertyDescriptor<TextBox, bool> AcceptsReturn			 { this, "AcceptsReturn", false, InvalidationKind::Measure };

		EventSignal<> TextChanged;

		TextBox();

		void OnPropertyChanged(const char* propertyName) override;

		void OnKeyDown(InputEvent input) override;
		void OnKeyUp(InputEvent input) override;

		bool MoveFocusNext(Direction direction, InputModifier modifiers = InputModifier::None) override;
		void OnGotFocus() override;
		void OnLostFocus() override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}