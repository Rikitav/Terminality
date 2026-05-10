export module terminality:Button;

import std;
import :Geometry;
import :ControlBase;
import :EventSignal;
import :PropertyDescriptor;
import :RenderContext;

export namespace terminality
{
	class Button : public ControlBase
	{
		bool isPressed_ = false;

	public:
		Property<Button, std::wstring> Text  { this, "Text", L"", InvalidationKind::Measure };
		Property<Button, Color> PressedForegroundColor { this, "PressedForegroundColor", Color::BLACK, InvalidationKind::Visual };
		Property<Button, Color> PressedBackgroundColor { this, "PressedBackgroundColor", Color::CYAN, InvalidationKind::Visual };

		Event<> Clicked;

		void Click();

		void OnPropertyChanged(const char* propertyName) override;

		void OnLostFocus() override;

		bool OnKeyDown(InputEvent input) override;
		bool OnKeyUp(InputEvent input) override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}

