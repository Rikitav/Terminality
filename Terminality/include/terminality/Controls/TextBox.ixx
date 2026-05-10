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
		std::size_t cursorPosition_ = 0;

	public:
		Property<TextBox, std::wstring> Text		 { this, "Text", L"", InvalidationKind::Measure };
		Property<TextBox, TextWrap> TextWrapping	 { this, "TextWrapping", terminality::TextWrap::NoWrap, InvalidationKind::Measure };
		Property<TextBox, TextAlign> TextAlignment { this, "TextAlignment", terminality::TextAlign::Left, InvalidationKind::Visual };
		Property<TextBox, bool> AcceptsReturn		 { this, "AcceptsReturn", false, InvalidationKind::Measure };

		Event<> TextChanged;

		TextBox();

		void OnPropertyChanged(const char* propertyName) override;

		bool OnKeyDown(InputEvent input) override;
		bool OnKeyUp(InputEvent input) override;

		bool MoveFocusNext(Direction direction, InputModifier modifiers = InputModifier::None) override;
		void OnGotFocus() override;
		void OnLostFocus() override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}