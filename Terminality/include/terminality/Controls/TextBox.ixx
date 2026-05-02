export module terminality:TextBox;

import std;
import :Geometry;
import :ControlBase;
import :EventSignal;
import :RenderContext;
import :InputEvent;

export namespace terminality
{
	class TextBox : public ControlBase
	{
		std::wstring text_;
		size_t cursorPosition_ = 0;

	public:
		TextBox();

		EventSignal<> TextChanged;

		std::wstring GetText() const;
		void SetText(std::wstring text);

		void OnKeyDown(InputEvent input) override;
		void OnKeyUp(InputEvent input) override;

		bool MoveFocusNext(Direction direction, InputModifier modifiers = InputModifier::None) override;
		void OnGotFocus() override;
		void OnLostFocus() override;

	protected:
		Size Measure(const Size& availableSize) override;
		void Render(RenderContext& context) override;
	};
}