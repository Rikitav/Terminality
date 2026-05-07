export module terminality:VisualTreeNode;

import :Focus;
import :Geometry;
import :InputEvent;
import :RenderContext;

export namespace terminality
{
	class VisualTreeNode
	{
	protected:
		bool measureDirty_ = true;
		bool arrangeDirty_ = true;
		bool visualDirty_ = true;

		bool attached_ = false;
		bool focusable_ = true;
		bool focused_ = false;
		bool isTabStop_ = false;
		int tabIndex_ = 0;

		Size actualSize_;
		Rect arrangedRect_;

		VisualTreeNode* parent_ = nullptr;

		static void PopFocus(Direction direction, InputModifier modifiers);
		static void PushFocus(VisualTreeNode* focused);

	public:
		VisualTreeNode() = default;
		virtual ~VisualTreeNode() = default;

		// Copy and move semantics
		VisualTreeNode(const VisualTreeNode&) = delete;
		VisualTreeNode& operator=(const VisualTreeNode&) = delete;

		// Tree structure
		VisualTreeNode* GetParent() const;
		virtual void SetParent(VisualTreeNode* parent) = 0;
		virtual const std::span<VisualTreeNode*> GetChildren() const = 0;

		// Layout
		void InvalidateMeasure();
		void InvalidateArrange();
		void InvalidateVisual();

		virtual Size Measure(const Size& availableSize) = 0;
		virtual void Arrange(const Rect& finalRect) = 0;
		virtual void Render(RenderContext& context) = 0;

		// Getters
		bool IsAttached() const;
		bool IsMeasureDirty() const;
		bool IsArrangeDirty() const;
		bool IsVisualDirty() const;

		virtual bool IsFocusable() const;
		virtual bool IsTabStop() const;
		virtual int GetTabIndex() const;

		// Setters
		virtual void SetFocusable(bool value) = 0;
		virtual void SetTabStop(bool value) = 0;
		virtual void SetTabIndex(int value) = 0;

		// User input
		virtual bool OnKeyDown(InputEvent input) = 0;
		virtual bool OnKeyUp(InputEvent input) = 0;

		// Focus management
		virtual bool MoveFocusNext(Direction direction, InputModifier modifiers = InputModifier::None);
		virtual void OnGotFocus();
		virtual void OnLostFocus();

		// Tree invalidation
		virtual void OnChildInvalidated(VisualTreeNode& child);
		virtual void OnAttachedToTree();
		virtual void OnDettachedFromTree();
	};
}