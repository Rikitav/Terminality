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
		std::vector<std::unique_ptr<ControlBase>> contents_;
		size_t focusedIndex_ = 0;

	public:
		StackPanel() = default;

		void AddChild(std::unique_ptr<ControlBase> child);
		std::unique_ptr<ControlBase> RemoveChild(ControlPredicate predicate);

		void OnPropertyChanged(const char* propertyName) override;

		bool MoveFocusNext(Direction direction, InputModifier modifiers) override;
		void OnGotFocus() override;
		void OnLostFocus() override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}