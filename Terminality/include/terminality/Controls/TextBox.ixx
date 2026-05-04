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
		std::wstring text_;
		size_t cursorPosition_ = 0;
		TextWrapping textWrapping_ = TextWrapping::NoWrap;
		TextAlignment textAlignment_ = TextAlignment::Left;
		bool acceptsReturn_ = false;

	public:
		TextBox();

		EventSignal<> TextChanged;

		std::wstring GetText() const;
		void SetText(std::wstring text);

		TextWrapping GetTextWrapping() const;
		void SetTextWrapping(TextWrapping textWrapping);

		TextAlignment GetTextAlignment() const;
		void SetTextAlignment(TextAlignment textAlignment);

		bool GetAcceptsReturn() const;
		void SetAcceptsReturn(bool acceptsReturn);

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