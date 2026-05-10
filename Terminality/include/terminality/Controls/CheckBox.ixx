export module terminality:CheckBox;

import std;
import :Geometry;
import :ControlBase;
import :EventSignal;
import :PropertyDescriptor;
import :RenderContext;

export namespace terminality
{
	class CheckBox : public ControlBase
	{
		bool isPressed_ = false;
		std::optional<bool> isChecked_ = false;

	public:
		Property<CheckBox, std::wstring> Text{ this, "Text", L"", InvalidationKind::Measure };
		Property<CheckBox, Color> PressedForegroundColor{ this, "PressedForegroundColor", Color::BLACK, InvalidationKind::Visual };
		Property<CheckBox, Color> PressedBackgroundColor{ this, "PressedBackgroundColor", Color::CYAN, InvalidationKind::Visual };

		Event<std::optional<bool>> Toggled;
		Event<> Checked;
		Event<> Unchecked;

		void Toggle(std::optional<bool> value);

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

