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
		PropertyDescriptor<CheckBox, std::wstring> Text{ this, "Text", L"", InvalidationKind::Measure };
		PropertyDescriptor<CheckBox, Color> PressedForegroundColor{ this, "PressedForegroundColor", Color::BLACK, InvalidationKind::Visual };
		PropertyDescriptor<CheckBox, Color> PressedBackgroundColor{ this, "PressedBackgroundColor", Color::CYAN, InvalidationKind::Visual };

		EventSignal<std::optional<bool>> Toggled;
		EventSignal<> Checked;
		EventSignal<> Unchecked;

		void Toggle(std::optional<bool> value);

		void OnPropertyChanged(const char* propertyName) override;

		void OnLostFocus() override;

		void OnKeyDown(InputEvent input) override;
		void OnKeyUp(InputEvent input) override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}

