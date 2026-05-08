export module terminality:VisualTree;

import std;
import std.compat;
import :Geometry;
import :VisualTreeNode;
import :RenderBuffer;
import :FocusManager;

export namespace terminality
{
	struct UILayer
	{
		std::unique_ptr<VisualTreeNode> RootNode;
		FocusManager Focus;
	};

	class VisualTree
	{
		std::vector<UILayer> layers_;
		bool hasDirtyVisual_ = true;
		Rect dirtyRect_;

		VisualTree() = default;
		void CollectDirtyNodeRect(const VisualTreeNode& node);

	public:
		static VisualTree& Current();

		VisualTreeNode* Root() const;
		VisualTreeNode* PeekLayer() const;
		void PushLayer(std::unique_ptr<VisualTreeNode> layerRoot);
		void PopLayer();
		
		FocusManager& GetFocusManager();

		void Invalidate(const Rect& dirtyRect);
		bool HasDirtyVisual() const { return hasDirtyVisual_; }

		void RunLayout(const Size& viewportSize);
		void Render(RenderBuffer& buffer);
	};
}
