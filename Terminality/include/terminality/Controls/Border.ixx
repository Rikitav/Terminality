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
		std::unique_ptr<ControlBase> content_ = nullptr;
		Color borderColor_ = Color::DARK_GRAY;
		Thickness borderThickness_ = Thickness(1, 1, 1, 1);

	public:
		Color GetBorderColor() const;
		//Thickness GetBorderThickness() const;
		ControlBase* GetContent() const;

		void SetBorderColor(Color color);
		//void SetBorderThickness(Thickness thickness);
		void SetContent(std::unique_ptr<ControlBase> child);

		bool MoveFocusNext(Direction direction, InputModifier modifiers = InputModifier::None) override;
		void OnGotFocus() override;
		void OnLostFocus() override;

		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}
