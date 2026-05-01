export module terminality:ControlBase;

import std;
import :Geometry;
import :Layout;
import :RenderContext;
import :DependencyObject;
import :VisualTreeNode;

export namespace terminality
{
	class ControlBase : public DependencyObject
	{
	public:
		// Setters
		void SetParent(VisualTreeNode* parent) override;

		void SetSize(Size value) override;
		void SetMinSize(Size value) override;
		void SetMaxSize(Size value) override;

		void SetMargin(Thickness value) override;
		void SetHorizontalAlignment(HorizontalAlignment value) override;
		void SetVerticalAlignment(VerticalAlignment value) override;

		void SetFocusable(bool value) override;
		void SetTabStop(bool value) override;
		void SetTabIndex(int value) override;
		
		// Layout
		Size Measure(const Size& availableSize) override;
		void Arrange(const Rect& finalRect) override;
		void Render(RenderContext& context) override;

		virtual const std::span<VisualTreeNode*> GetChildren() const;
	};

	typedef std::function<bool(const ControlBase*)> ControlPredicate;
}
