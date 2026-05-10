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
		size_t focusedIndex_ = 0;

	public:
		PropertyDescriptor<StackPanel, Orientation> ContentOrientation								{ this, "ContentOrientation", Orientation::Vertical, InvalidationKind::Measure };
		PropertyDescriptor<StackPanel, terminality::HorizontalAlignment> HorizontalContentAlignment { this, "HorizontalContentAlignment", HorizontalAlignment::Stretch, InvalidationKind::Measure };
		PropertyDescriptor<StackPanel, terminality::VerticalAlignment> VerticalContentAlignment	    { this, "VerticalContentAlignment", VerticalAlignment::Stretch, InvalidationKind::Measure };
		PropertyDescriptor<StackPanel, bool> Looping												{ this, "Looping", false, InvalidationKind::None };

		StackPanel() = default;

		void AddChild(std::unique_ptr<ControlBase> child);
		void Insert(size_t index, std::unique_ptr<ControlBase> child);
		std::unique_ptr<ControlBase> RemoveChild(ControlPredicate predicate);
		std::unique_ptr<ControlBase> RemoveAt(size_t index);
		void Clear();

		void OnPropertyChanged(const char* propertyName) override;

		bool MoveFocusNext(Direction direction, InputModifier modifiers) override;
		void OnGotFocus() override;
		void OnLostFocus() override;

		size_t VisualChildrenCount() const override;
		VisualTreeNode* GetVisualChild(size_t index) const override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}