export module terminality:VisualTree;

import std;
import std.compat;
import :VisualTreeNode;
import :RenderBuffer;

export namespace terminality
{
	class VisualTree
	{
		std::unique_ptr<VisualTreeNode> rootNode_;
		bool hasDirtyVisual_ = true;
		Rect dirtyRect_;

		void CollectDirtyNodeRect(const VisualTreeNode& node);

	public:
		void SetRoot(std::unique_ptr<VisualTreeNode> rootNode);
		VisualTreeNode* Root() const { return rootNode_.get(); }

		void Invalidate(const Rect& dirtyRect);
		bool HasDirtyVisual() const { return hasDirtyVisual_; }

		void RunLayout(const Size& viewportSize);
		void Render(RenderBuffer& buffer);
	};
}
