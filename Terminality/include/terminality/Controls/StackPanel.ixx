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

		bool MoveFocusNext(NavigationDirection direction) override;
		void OnGotFocus() override;
		void OnLostFocus() override;

		Size Measure(const Size& availableSize) override;
		void Arrange(const Rect& contentRect) override;
		void Render(RenderContext& context) override;
	};
}