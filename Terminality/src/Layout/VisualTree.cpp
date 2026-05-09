module terminality;

import std;

using namespace terminality;

VisualTree::VisualTree()
{
	layers_.reserve(100);
}

VisualTree& VisualTree::Current()
{
	if (HostApplication::IsUiThread())
		throw std::runtime_error("Cannot get FocusManager within running UI thread or Before UI thread was started.");

	static VisualTree visualTree;
	return visualTree;
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
	hasDirtyVisual_ = true;
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

		hasDirtyVisual_ = true;
		dirtyRect_ = Rect();
	}
}

FocusManager& VisualTree::GetFocusManager()
{
	if (layers_.empty())
		throw std::runtime_error("No layers in VisualTree");

	return layers_.back()->Focus;
}

void VisualTree::Invalidate(const Rect& dirtyRect)
{
	hasDirtyVisual_ = true;
	dirtyRect_ = Rect::Union(dirtyRect_, dirtyRect);
}

void VisualTree::CollectDirtyNodeRect(const VisualTreeNode& node)
{
	if (node.IsVisualDirty())
	{
		hasDirtyVisual_ = true;
	}
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

void VisualTree::Render(RenderBuffer& buffer)
{
	if (layers_.empty())
		return;

	if (!hasDirtyVisual_)
		return;

	UILayer& topLayer = *layers_.back();
	if (topLayer.RootNode == nullptr)
		return;

	Rect nodeRect = topLayer.RootNode.get()->GetArrangedRect();
	RenderContext context(buffer, nodeRect);

	topLayer.RootNode->Render(context);
	hasDirtyVisual_ = false;
	dirtyRect_ = Rect();
}
