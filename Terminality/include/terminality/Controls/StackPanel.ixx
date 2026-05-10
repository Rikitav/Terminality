export module terminality:StackPanel;

import std;
import std.compat;
import :Focus;
import :Geometry;
import :RenderContext;
import :ControlBase;

export namespace terminality
{
	class StackPanel : public ControlBase
	{
	protected:
		std::vector<std::unique_ptr<ControlBase>> contents_;
		std::size_t focusedIndex_ = 0;

	public:
		Property<StackPanel, Orientation> ContentOrientation								{ this, "ContentOrientation", Orientation::Vertical, InvalidationKind::Measure };
		Property<StackPanel, terminality::HorizaontalAllign> HorizontalContentAlignment { this, "HorizontalContentAlignment", HorizaontalAllign::Stretch, InvalidationKind::Measure };
		Property<StackPanel, terminality::VerticalAlign> VerticalContentAlignment	    { this, "VerticalContentAlignment", VerticalAlign::Stretch, InvalidationKind::Measure };
		Property<StackPanel, bool> Looping												{ this, "Looping", false, InvalidationKind::None };

		StackPanel() = default;

		void AddChild(std::unique_ptr<ControlBase> child);
		void Insert(std::size_t index, std::unique_ptr<ControlBase> child);
		std::unique_ptr<ControlBase> RemoveChild(ControlPredicate predicate);
		std::unique_ptr<ControlBase> RemoveAt(std::size_t index);
		void Clear();

		void OnPropertyChanged(const char* propertyName) override;

		bool MoveFocusNext(Direction direction, InputModifier modifiers) override;
		void OnGotFocus() override;
		void OnLostFocus() override;

		std::size_t VisualChildrenCount() const override;
		VisualTreeNode* GetVisualChild(std::size_t index) const override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}