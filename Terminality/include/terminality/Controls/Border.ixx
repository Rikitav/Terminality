export module terminality:Border;

import std;
import std.compat;
import :Focus;
import :Geometry;
import :ControlBase;

export namespace terminality
{
	class Border : public ControlBase
	{
	public:
		PropertyDescriptor<Border, Color> BorderColor					 { this, "BorderColor", Color::DARK_GRAY, InvalidationKind::Visual};
		PropertyDescriptor<Border, Thickness> BorderThickness			 { this, "BorderThickness", Thickness::Single, InvalidationKind::Visual };
		PropertyDescriptor<Border, std::wstring> HeaderText			     { this, "HeaderText", L"", InvalidationKind::Visual };
		PropertyDescriptor<Border, std::unique_ptr<ControlBase>> Content { this, "Content", nullptr, InvalidationKind::Visual };

		Border();
		Border(std::unique_ptr<ControlBase> content);

		void OnPropertyChanged(const char* propertyName) override;

		bool MoveFocusNext(Direction direction, InputModifier modifiers = InputModifier::None) override;
		void OnGotFocus() override;
		void OnLostFocus() override;

		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}
