
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>

#include <terminality/Controls/TreeView.hpp>

using namespace terminality;

// ===========================================================================
// TreeNode
// ===========================================================================

TreeNode::TreeNode(std::wstring header, std::wstring tag)
	: header_(std::move(header)), tag_(std::move(tag)) { }

void TreeNode::SetOwnerRecursive(TreeView* treeView)
{
	treeView_ = treeView;
	for (auto& child : children_)
		child->SetOwnerRecursive(treeView);
}

void TreeNode::NotifyChanged()
{
	if (treeView_ != nullptr)
		treeView_->OnModelChanged();
}

void TreeNode::SetHeader(std::wstring header)
{
	if (header_ == header)
		return;

	header_ = std::move(header);
	NotifyChanged();
}

const std::wstring& TreeNode::GetHeader() const
{
	return header_;
}

void TreeNode::SetTag(std::wstring tag)
{
	tag_ = std::move(tag);
}

const std::wstring& TreeNode::GetTag() const
{
	return tag_;
}

bool TreeNode::IsExpanded() const
{
	return expanded_;
}

bool TreeNode::HasChildren() const
{
	return !children_.empty();
}

void TreeNode::Expand()
{
	if (expanded_)
		return;

	expanded_ = true;
	Expanded.Emit();
	NotifyChanged();
}

void TreeNode::Collapse()
{
	if (!expanded_)
		return;

	expanded_ = false;
	Collapsed.Emit();
	NotifyChanged();
}

void TreeNode::ToggleExpanded()
{
	if (expanded_)
		Collapse();
	else
		Expand();
}

TreeNode* TreeNode::GetParent() const
{
	return parent_;
}

std::size_t TreeNode::GetDepth() const
{
	// Count ancestors up to (but excluding) the invisible TreeView root.
	std::size_t depth = 0;
	for (TreeNode* p = parent_; p != nullptr && p->parent_ != nullptr; p = p->parent_)
		++depth;

	return depth;
}

std::size_t TreeNode::ChildrenCount() const { return children_.size(); }

TreeNode* TreeNode::GetChild(std::size_t index) const
{
	return index < children_.size() ? children_[index].get() : nullptr;
}

TreeNode* TreeNode::AddChild(std::unique_ptr<TreeNode> child)
{
	if (!child)
		return nullptr;

	child->parent_ = this;
	child->SetOwnerRecursive(treeView_);

	TreeNode* raw = child.get();
	children_.push_back(std::move(child));

	NotifyChanged();
	return raw;
}

TreeNode* TreeNode::AddChild(std::wstring header, std::wstring tag)
{
	return AddChild(std::make_unique<TreeNode>(std::move(header), std::move(tag)));
}

std::unique_ptr<TreeNode> TreeNode::RemoveChild(TreeNode* child)
{
	for (auto it = children_.begin(); it != children_.end(); ++it)
	{
		if (it->get() == child)
		{
			auto removed = std::move(*it);
			removed->parent_ = nullptr;
			removed->SetOwnerRecursive(nullptr);

			children_.erase(it);

			NotifyChanged();
			return removed;
		}
	}

	return nullptr;
}

std::unique_ptr<TreeNode> TreeNode::RemoveAt(std::size_t index)
{
	if (index >= children_.size())
		return nullptr;

	auto it = children_.begin() + static_cast<std::ptrdiff_t>(index);
	auto removed = std::move(*it);
	removed->parent_ = nullptr;
	removed->SetOwnerRecursive(nullptr);

	children_.erase(it);

	NotifyChanged();
	return removed;
}

void TreeNode::Clear()
{
	for (auto& child : children_)
	{
		child->parent_ = nullptr;
		child->SetOwnerRecursive(nullptr);
	}

	children_.clear();
	NotifyChanged();
}

// ===========================================================================
// TreeView
// ===========================================================================

TreeView::TreeView()
{
	// The invisible root belongs to this view so that top-level nodes added
	// through it inherit the correct owner.
	root_.treeView_ = this;
}

void TreeView::OnModelChanged()
{
	if (suspendNotify_ > 0)
	{
		modelDirty_ = true;
		return;
	}

	RebuildVisible();
	EnsureSelectionValid();
	ScrollSelectedIntoView();
	InvalidateMeasure();
}

void TreeView::CollectVisible(TreeNode* node)
{
	visible_.push_back(node);
	if (node->expanded_ && !node->children_.empty())
	{
		for (auto& child : node->children_)
			CollectVisible(child.get());
	}
}

void TreeView::RebuildVisible()
{
	visible_.clear();
	for (auto& child : root_.children_)
		CollectVisible(child.get());
}

void TreeView::EnsureSelectionValid()
{
	if (visible_.empty())
	{
		selectedIndex_ = 0;
		return;
	}

	if (selectedIndex_ >= visible_.size())
		selectedIndex_ = visible_.size() - 1;
}

void TreeView::ScrollSelectedIntoView()
{
	if (visible_.empty())
	{
		scrollOffset_ = 0;
		return;
	}

	int32_t viewHeight = arrangedRect_.Height;
	if (viewHeight <= 0)
		viewHeight = static_cast<int32_t>(visible_.size());

	int32_t sel = static_cast<int32_t>(selectedIndex_);
	if (sel < scrollOffset_)
		scrollOffset_ = sel;
	else if (sel >= scrollOffset_ + viewHeight)
		scrollOffset_ = sel - viewHeight + 1;

	int32_t maxOffset = static_cast<int32_t>(visible_.size()) - viewHeight;
	if (maxOffset < 0)
		maxOffset = 0;

	scrollOffset_ = std::clamp(scrollOffset_, 0, maxOffset);
}

TreeNode* TreeView::AddNode(std::unique_ptr<TreeNode> node)
{
	return root_.AddChild(std::move(node));
}

TreeNode* TreeView::AddNode(std::wstring header, std::wstring tag)
{
	return root_.AddChild(std::move(header), std::move(tag));
}

std::unique_ptr<TreeNode> TreeView::RemoveNode(TreeNode* node)
{
	return root_.RemoveChild(node);
}

void TreeView::Clear()
{
	root_.Clear();
}

TreeNode* TreeView::GetRoot() { return &root_; }

std::size_t TreeView::NodeCount() const { return root_.children_.size(); }

TreeNode* TreeView::GetSelected() const
{
	if (visible_.empty() || selectedIndex_ >= visible_.size())
		return nullptr;

	return visible_[selectedIndex_];
}

bool TreeView::SelectIndex(std::size_t index, bool emitEvent)
{
	if (visible_.empty())
	{
		selectedIndex_ = 0;
		return false;
	}

	if (index >= visible_.size())
		return false;

	if (index == selectedIndex_)
		return true;

	selectedIndex_ = index;
	ScrollSelectedIntoView();
	InvalidateVisual();

	if (emitEvent)
		SelectionChanged.Emit(visible_[selectedIndex_]);

	return true;
}

bool TreeView::Select(TreeNode* node)
{
	if (node == nullptr || node == &root_)
		return false;

	// Expand ancestors so the requested node becomes visible.
	for (TreeNode* ancestor = node->parent_;
		 ancestor != nullptr && ancestor != &root_;
		 ancestor = ancestor->parent_)
	{
		ancestor->Expand();
	}

	for (std::size_t i = 0; i < visible_.size(); ++i)
	{
		if (visible_[i] == node)
			return SelectIndex(i, true);
	}

	return false;
}

void TreeView::SelectFirst()
{
	if (!visible_.empty())
		SelectIndex(0, true);
}

void TreeView::SelectLast()
{
	if (!visible_.empty())
		SelectIndex(visible_.size() - 1, true);
}

void TreeView::SetRecursiveExpanded(TreeNode* node, bool expanded)
{
	if (!node->children_.empty())
	{
		if (expanded)
			node->Expand();
		else
			node->Collapse();
	}

	for (auto& child : node->children_)
		SetRecursiveExpanded(child.get(), expanded);
}

void TreeView::ExpandAll()
{
	NotifyGuard guard(*this);
	for (auto& child : root_.children_)
		SetRecursiveExpanded(child.get(), true);
}

void TreeView::CollapseAll()
{
	NotifyGuard guard(*this);
	for (auto& child : root_.children_)
		SetRecursiveExpanded(child.get(), false);
}

bool TreeView::OnKeyDown(InputEvent input)
{
	if (visible_.empty())
		return ControlBase::OnKeyDown(input);

	std::size_t last = visible_.size() - 1;
	TreeNode* selected = visible_[selectedIndex_];

	switch (input.Key)
	{
		case InputKey::UP:
		{
			if (selectedIndex_ > 0)
			{
				SelectIndex(selectedIndex_ - 1, true);
				return true;
			}
			break; // already at the top -> let focus leave the control
		}

		case InputKey::DOWN:
		{
			if (selectedIndex_ < last)
			{
				SelectIndex(selectedIndex_ + 1, true);
				return true;
			}
			break; // already at the bottom -> let focus leave the control
		}

		case InputKey::LEFT:
		{
			if (selected->IsExpanded())
			{
				selected->Collapse();
				return true;
			}

			TreeNode* parent = selected->GetParent();
			if (parent != nullptr && parent != &root_)
			{
				Select(parent);
				return true;
			}
			break;
		}

		case InputKey::RIGHT:
		{
			if (selected->HasChildren())
			{
				if (!selected->IsExpanded())
				{
					selected->Expand();
					return true;
				}

				// Already expanded: move to the first child (next visible row).
				if (selectedIndex_ < last)
				{
					SelectIndex(selectedIndex_ + 1, true);
					return true;
				}
			}
			break;
		}

		case InputKey::HOME:
		{
			SelectIndex(0, true);
			return true;
		}

		case InputKey::END:
		{
			SelectIndex(last, true);
			return true;
		}

		case InputKey::RETURN:
		case InputKey::SPACE:
		{
			NodeActivated.Emit(selected);
			return true;
		}

		default:
			break;
	}

	return ControlBase::OnKeyDown(input);
}

Size TreeView::MeasureOverride(const Size& availableSize)
{
	RebuildVisible();
	EnsureSelectionValid();

	int32_t height = static_cast<int32_t>(visible_.size());
	if (availableSize.Height >= 0)
		height = std::min(height, availableSize.Height);

	const std::size_t indent = IndentSize.Get();
	const int32_t glyphColumn = ShowGlyphs.Get() ? 2 : 0; // glyph + trailing space

	int32_t maxWidth = 0;
	for (TreeNode* node : visible_)
	{
		int32_t width = static_cast<int32_t>(node->GetDepth() * indent)
			+ glyphColumn
			+ static_cast<int32_t>(node->GetHeader().size());

		maxWidth = std::max(maxWidth, width);
	}

	int32_t width = availableSize.Width >= 0 ? std::min(availableSize.Width, maxWidth) : maxWidth;
	if (width < 0)
		width = 0;

	return Size(width, height);
}

void TreeView::ArrangeOverride(const Rect& /*contentRect*/)
{
	ScrollSelectedIntoView();
}

void TreeView::RenderOverride(RenderContext& context)
{
	const Rect rect = context.ContextRect();
	if (rect.Height <= 0 || visible_.empty())
		return;

	ScrollSelectedIntoView();

	const std::size_t indent = IndentSize.Get();
	const bool showGlyphs = ShowGlyphs.Get();
	const wchar_t collapsedGlyph = CollapsedGlyph.Get();
	const wchar_t expandedGlyph = ExpandedGlyph.Get();
	const wchar_t leafGlyph = LeafGlyph.Get();

	const std::size_t start = static_cast<std::size_t>(scrollOffset_ < 0 ? 0 : scrollOffset_);
	const std::size_t end = std::min(visible_.size(), start + static_cast<std::size_t>(rect.Height));

	for (std::size_t i = start; i < end; ++i)
	{
		TreeNode* node = visible_[i];
		int32_t y = static_cast<int32_t>(i) - scrollOffset_;

		std::wstring line;
		line.append(node->GetDepth() * indent, L' ');

		if (showGlyphs)
		{
			wchar_t glyph = node->HasChildren()
				? (node->IsExpanded() ? expandedGlyph : collapsedGlyph)
				: leafGlyph;

			line.push_back(glyph);
			line.push_back(L' ');
		}

		line += node->GetHeader();

		const bool isSelected = (i == selectedIndex_);
		Color fg = isSelected ? FocusedForegroundColor.Get() : ForegroundColor.Get();
		Color bg = isSelected ? FocusedBackgroundColor.Get() : BackgroundColor.Get();

		// Extend the selected row to full width so the highlight bar is solid.
		if (isSelected)
		{
			if (static_cast<int32_t>(line.size()) < rect.Width)
				line.append(static_cast<std::size_t>(rect.Width - static_cast<int32_t>(line.size())), L' ');
		}

		if (rect.Width > 0 && static_cast<int32_t>(line.size()) > rect.Width)
			line.resize(static_cast<std::size_t>(rect.Width));

		context.RenderText(Point(0, y), line, fg, bg, false);
	}
}
