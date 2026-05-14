export module terminality:Border;

import <cstdint>;
import <string>;
import <memory>;

import :Color;
import :Property;
import :Focus;
import :Geometry;
import :ControlBase;

export namespace terminality
{
	enum class RectanglePos
	{
		LeftTopCorner,
		LeftBottomCorner,
		
		RightTopCorner,
		RightBottomCorner,
		
		LeftVerticalLine,
		RightVerticalLine,
		
		TopHorizontalLine,
		BottomHorizontalLine
	};

	typedef wchar_t (*BorderStyle)(const RectanglePos pos);

	class Border : public ControlBase
	{
	public:
		Property<Border, Color> BorderColor					   { this, "BorderColor", Color::DARK_GRAY, InvalidationKind::Visual};
		Property<Border, Color> FocusedBorderColor             { this, "FocusedBorderColor", Color::CYAN, InvalidationKind::Visual };
		Property<Border, Thickness> BorderThickness			   { this, "BorderThickness", Thickness::Single, InvalidationKind::Visual };
		Property<Border, std::wstring> HeaderText			   { this, "HeaderText", L"", InvalidationKind::Visual };
		Property<Border, std::unique_ptr<ControlBase>> Content { this, "Content", nullptr, InvalidationKind::Visual };
		Property<Border, BorderStyle> Style				       { this, "BorderStyle", nullptr, InvalidationKind::Visual };

		Border();
		Border(std::unique_ptr<ControlBase> content);

		void OnPropertyChanged(const char* propertyName) override;

		bool MoveFocusNext(Direction direction, InputModifier modifiers = InputModifier::None) override;
		void OnGotFocus() override;
		void OnLostFocus() override;

		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;

		std::size_t VisualChildrenCount() const override;
		VisualTreeNode* GetVisualChild(std::size_t index) const override;
	};
}
