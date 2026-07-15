#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <atomic>
#include <vector>
#include <optional>

#include <terminality/Core/Geometry.hpp>
#include <terminality/Framework/VisualTreeNode.hpp>
#include <terminality/Engine/RenderBuffer.hpp>
#include <terminality/Engine/FocusManager.hpp>

namespace terminality
{
	struct UILayer
	{
		std::unique_ptr<VisualTreeNode> RootNode;
		std::atomic<bool> Running { false };
		FocusManager Focus;

		UILayer() = delete;

		UILayer(std::unique_ptr<VisualTreeNode> rootNode) : RootNode(std::move(rootNode))
		{
			RootNode->SetLayer(this);
		}

		UILayer(const UILayer&) = delete;
		UILayer& operator=(const UILayer&) = delete;
		UILayer(UILayer&&) = delete;
		UILayer& operator=(UILayer&&) = delete;
	};

	class VisualTree
	{
		std::vector<std::unique_ptr<UILayer>> layers_;
		std::optional<Rect> dirtyRect_;

		VisualTree();
		VisualTree(const VisualTree&) = delete;
		VisualTree& operator=(const VisualTree&) = delete;

		void CollectDirtyNodeRect(const VisualTreeNode& node);

	public:
		static VisualTree& Current();

		std::size_t LayerCount();
		VisualTreeNode* Root() const;
		VisualTreeNode* PeekLayer() const;
		UILayer& PushLayer(std::unique_ptr<VisualTreeNode> layerRoot);
		void PopLayer();

		FocusManager& GetFocusManager();

		void Invalidate(const Rect& dirtyRect);
		bool HasDirtyVisual() const { return dirtyRect_.has_value(); }

		void RunLayout(const Size& viewportSize);
		void Render(RenderBuffer& buffer);
	};
}
