
#include <cstdint>
#include <memory>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <stack>

#include <terminality/Framework/VisualTree.hpp>
#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Engine/DispatchTimer.hpp>

using namespace terminality;

namespace
{
	// A layer counts as covering the viewport only if it actually paints every
	// cell of it: the root must be arranged over the viewport AND have a
	// non-transparent background. Layer roots are always arranged into the
	// full viewport rect by RunLayout, so the arranged rect alone cannot tell
	// a painted full screen apart from a transparent overlay root (e.g. a
	// centered dialog whose root stretches over the viewport but only draws
	// the dialog itself) — the background color decides.
	bool LayerCoversViewport(const UILayer& layer, const Rect& viewportRect)
	{
		const VisualTreeNode* root = layer.RootNode.get();
		if (root == nullptr)
			return false;

		if (!root->GetArrangedRect().Contains(viewportRect))
			return false;

		const ControlBase* control = dynamic_cast<const ControlBase*>(root);
		if (control == nullptr)
			return true; // non-ControlBase root: assume opaque

		return control->IsVisible && control->GetEffectiveBackgroundColor() != Color::TRANSPARENT;
	}
}

VisualTree::VisualTree()
{
	layers_.reserve(100);
}

VisualTree& VisualTree::Current()
{
	if (!DispatchTimer::Current().CheckAccess())
		throw std::runtime_error("Cannot get FocusManager within running UI thread or Before UI thread was started.");

	static VisualTree visualTree;
	return visualTree;
}

size_t VisualTree::LayerCount() const
{
	return layers_.size();
}

VisualTreeNode* VisualTree::Root() const
{
	if (layers_.empty())
		return nullptr;

	return layers_.at(0)->RootNode.get();
}

VisualTreeNode* VisualTree::PeekLayer() const
{
	if (layers_.empty())
		return nullptr;

	return layers_.back()->RootNode.get();
}

/*
void VisualTree::SetRoot(std::unique_ptr<VisualTreeNode> rootNode)
{
	if (layers_.empty())
	{
		layers_.push_back(UILayer{ std::move(rootNode), FocusManager() });
	}
	else
	{
		layers_[0] = UILayer{ std::move(rootNode), FocusManager() };
	}

	hasDirtyVisual_ = true;
	dirtyRect_ = Rect();
}
*/

UILayer& VisualTree::PushLayer(std::unique_ptr<VisualTreeNode> layerRoot)
{
	if (layers_.size() == 100)
		throw std::runtime_error("UI layer stack overflow.");

	layers_.emplace_back(std::make_unique<UILayer>(std::move(layerRoot)));
	dirtyRect_ = Rect();

	UILayer& layer = *layers_.back().get();
	layer.Focus.SetFocused(layer.RootNode.get());
	return layer;
}

void VisualTree::PopLayer()
{
	if (layers_.size() > 1)
	{
		UILayer& layer = *layers_.back().get();
		layer.Running.store(false);
		layers_.pop_back();

		dirtyRect_ = Rect();
	}
}

FocusManager& VisualTree::GetFocusManager()
{
	if (layers_.empty())
		throw std::runtime_error("No layers in VisualTree");

	return layers_.back()->Focus;
}

void VisualTree::StopCurrentLayer()
{
	if (!layers_.empty())
		layers_.back()->Running.store(false);
}

void VisualTree::StopNestedLayers()
{
	for (std::size_t i = 1; i < layers_.size(); ++i)
		layers_[i]->Running.store(false);
}

void VisualTree::Invalidate(const Rect& dirtyRect)
{
	if (!dirtyRect_)
		dirtyRect_ = dirtyRect;
	else
		dirtyRect_ = Rect::Union(*dirtyRect_, dirtyRect);
}

void VisualTree::CollectDirtyNodeRect(const VisualTreeNode& node)
{
	if (node.IsVisualDirty() && !dirtyRect_)
		dirtyRect_ = Rect();
}

void VisualTree::RunLayout(const Size& viewportSize)
{
	for (auto& layer : layers_)
	{
		if (layer->RootNode == nullptr)
			continue;

		Size desiredSize = layer->RootNode->Measure(viewportSize);
		layer->RootNode->Arrange(Rect(0, 0, viewportSize.Width, viewportSize.Height));
		CollectDirtyNodeRect(*layer->RootNode);
	}
}

void VisualTree::RenderLayer(UILayer& layer, RenderBuffer& buffer)
{
	if (layer.RootNode == nullptr)
		return;

	Rect nodeRect = layer.RootNode.get()->GetArrangedRect();
	RenderContext context(buffer, nodeRect);

	layer.RootNode->Render(context);
}

void VisualTree::Render(RenderBuffer& buffer)
{
	if (layers_.empty())
		return;

	if (!dirtyRect_)
		return;

	const Rect viewportRect(0, 0, static_cast<int32_t>(buffer.Width()), static_cast<int32_t>(buffer.Height()));
	std::size_t bottom = 0;

	// Composite: find the oldest layer that covers the whole viewport and
	// render from it up to the newest. Layers below it are fully occluded and
	// are skipped; if no layer covers the viewport, render all.
	for (std::size_t i = layers_.size(); i-- > 0; )
	{
		if (LayerCoversViewport(*layers_[i], viewportRect))
		{
			bottom = i;
			break;
		}
	}

	for (std::size_t i = bottom; i < layers_.size(); ++i)
		RenderLayer(*layers_[i], buffer);

	dirtyRect_.reset();
}
