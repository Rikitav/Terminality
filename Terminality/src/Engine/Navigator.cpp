
#include <cstdint>
#include <memory>
#include <functional>

#include <terminality/Engine/Navigator.hpp>
#include <terminality/Framework/HostApplication.hpp>

using namespace terminality;

Navigator& Navigator::Current()
{
	static Navigator navigator;
	return navigator;
}

void Navigator::Navigate(std::unique_ptr<VisualTreeNode> page)
{
	if (!page)
		return;

	HostApplication& host = HostApplication::Current();
	VisualTree& tree = VisualTree::Current();
	
	UILayer& layer = tree.PushLayer(std::move(page));
	struct LayerGuard
	{
		VisualTree* tree;
		~LayerGuard() { if (tree) tree->PopLayer(); }
	} guard{ &tree };

	host.NestUILoop(layer);

	if (tree.Root())
	{
		tree.Root()->InvalidateMeasure();
		tree.Root()->InvalidateVisual();
	}
}

bool Navigator::CanGoBack() const
{
	return VisualTree::Current().LayerCount() > 1;
}

bool Navigator::GoBack()
{
	if (!CanGoBack())
		return false;

	VisualTree& tree = VisualTree::Current();
	tree.PopLayer();

	if (tree.PeekLayer() != nullptr)
	{
		tree.PeekLayer()->InvalidateMeasure();
		tree.PeekLayer()->InvalidateVisual();
	}

	return true;
}

void Navigator::GoHome()
{
	VisualTree& tree = VisualTree::Current();
	while (tree.LayerCount() > 1)
		tree.PopLayer();

	if (tree.Root())
	{
		tree.Root()->InvalidateMeasure();
		tree.Root()->InvalidateVisual();
	}
}