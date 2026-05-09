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
		std::atomic<bool> Running { false };
		FocusManager Focus;

		UILayer() = delete;

		UILayer(std::unique_ptr<VisualTreeNode> rootNode) : RootNode(std::move(rootNode))
		{
			RootNode->SetParent(nullptr, this);
		}

		UILayer(const UILayer&) = delete;
		UILayer& operator=(const UILayer&) = delete;
		UILayer(UILayer&&) = delete;
		UILayer& operator=(UILayer&&) = delete;
	};

	class VisualTree
	{
		std::vector<std::unique_ptr<UILayer>> layers_;
		bool hasDirtyVisual_ = true;
		Rect dirtyRect_;

		VisualTree();
		VisualTree(const VisualTree&) = delete;
		VisualTree& operator=(const VisualTree&) = delete;

		void CollectDirtyNodeRect(const VisualTreeNode& node);

	public:
		static VisualTree& Current();

		VisualTreeNode* Root() const;
		VisualTreeNode* PeekLayer() const;
		UILayer& PushLayer(std::unique_ptr<VisualTreeNode> layerRoot);
		void PopLayer();
		
		FocusManager& GetFocusManager();

		void Invalidate(const Rect& dirtyRect);
		bool HasDirtyVisual() const { return hasDirtyVisual_; }

		void RunLayout(const Size& viewportSize);
		void Render(RenderBuffer& buffer);
	};
}
