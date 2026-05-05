module terminality;

import std;

using namespace terminality;

VisualTree& VisualTree::Current()
{
	if (HostApplication::IsUiThread())
		throw std::runtime_error("Cannot get FocusManager within running UI thread or Before UI thread was started.");

	static VisualTree visualTree;
	return visualTree;
}

void VisualTree::SetRoot(std::unique_ptr<VisualTreeNode> rootNode)
{
	rootNode_ = std::move(rootNode);
	hasDirtyVisual_ = true;
	dirtyRect_ = Rect();
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
	if (rootNode_ == nullptr)
	{
		return;
	}

	Size desiredSize = rootNode_->Measure(viewportSize);
	rootNode_->Arrange(Rect(0, 0, viewportSize.Width, viewportSize.Height));
	CollectDirtyNodeRect(*rootNode_);
}

void VisualTree::Render(RenderBuffer& buffer)
{
	if (!rootNode_)
		return;

	if (!hasDirtyVisual_)
		return;

	RenderContext context(buffer, Rect(0, 0, buffer.Width(), buffer.Height()));
	rootNode_->Render(context);
	hasDirtyVisual_ = false;
	dirtyRect_ = Rect();
}
