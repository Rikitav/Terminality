#pragma once

#include <cstddef>
#include <vector>
#include <memory>
#include <string>

#include <terminality/Core/Color.hpp>
#include <terminality/Core/Geometry.hpp>
#include <terminality/Core/Layout.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Core/Focus.hpp>
#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Framework/Event.hpp>
#include <terminality/Framework/Property.hpp>
#include <terminality/Engine/RenderContext.hpp>

namespace terminality
{
	class TreeView;

	/// Hierarchical data node. Can be built and mutated on its own, outside of any
	/// TreeView. Once attached (directly or transitively) to a TreeView, every
	/// structural / expansion / text change is reported back so the view can refresh.
	class TreeNode
	{
		friend class TreeView;

		std::wstring header_;
		std::wstring tag_;
		bool expanded_ = false;

		TreeNode* parent_ = nullptr;
		TreeView* treeView_ = nullptr;
		std::vector<std::unique_ptr<TreeNode>> children_;

		// Recursively propagates the owning view (or nullptr to detach) to the
		// whole subtree without firing notifications.
		void SetOwnerRecursive(TreeView* treeView);

		// Notifies the owning view (if any) that the model changed.
		void NotifyChanged();

	public:
		// Emitted right after the node becomes expanded / collapsed.
		Event<> Expanded;
		Event<> Collapsed;

		explicit TreeNode(std::wstring header = L"", std::wstring tag = L"");
		~TreeNode() = default;

		TreeNode(const TreeNode&) = delete;
		TreeNode& operator=(const TreeNode&) = delete;

		// --- Content ---
		void SetHeader(std::wstring header);
		const std::wstring& GetHeader() const;

		void SetTag(std::wstring tag);
		const std::wstring& GetTag() const;

		// --- Expansion ---
		bool IsExpanded() const;
		bool HasChildren() const;
		void Expand();
		void Collapse();
		void ToggleExpanded();

		// --- Hierarchy (read-only navigation) ---
		TreeNode* GetParent() const;
		std::size_t GetDepth() const;        // 0 for top-level nodes
		std::size_t ChildrenCount() const;
		TreeNode* GetChild(std::size_t index) const;

		// --- Mutation ---
		// Returns a raw pointer to the added node so callers can keep building:
		//     auto* child = node->AddChild(L"child");
		//     child->AddChild(L"grandchild");
		TreeNode* AddChild(std::unique_ptr<TreeNode> child);
		TreeNode* AddChild(std::wstring header, std::wstring tag = L"");

		// Detaches the child (and its subtree) and returns ownership. The removed
		// subtree keeps its internal shape and can be re-parented elsewhere.
		std::unique_ptr<TreeNode> RemoveChild(TreeNode* child);
		std::unique_ptr<TreeNode> RemoveAt(std::size_t index);
		void Clear();
	};

	/// Renders a TreeNode hierarchy as an indented, keyboard-navigable list.
	class TreeView : public ControlBase
	{
		friend class TreeNode;

		// The invisible root that holds the user-supplied top-level nodes.
		TreeNode root_;

		// Flattened, depth-first projection of the currently visible nodes.
		std::vector<TreeNode*> visible_;
		std::size_t selectedIndex_ = 0;
		int32_t scrollOffset_ = 0;

		// Batched-notification support so bulk operations (ExpandAll, ...) only
		// rebuild the projection once.
		int suspendNotify_ = 0;
		bool modelDirty_ = false;

		void CollectVisible(TreeNode* node);
		void RebuildVisible();
		void EnsureSelectionValid();
		void ScrollSelectedIntoView();

		void OnModelChanged();
		bool SelectIndex(std::size_t index, bool emitEvent);
		void SetRecursiveExpanded(TreeNode* node, bool expanded);

		struct NotifyGuard
		{
			TreeView& owner;
			explicit NotifyGuard(TreeView& o) : owner(o) { owner.suspendNotify_++; }
			~NotifyGuard()
			{
				if (--owner.suspendNotify_ == 0 && owner.modelDirty_)
				{
					owner.modelDirty_ = false;
					owner.RebuildVisible();
					owner.EnsureSelectionValid();
					owner.ScrollSelectedIntoView();
					owner.InvalidateMeasure();
				}
			}
		};

	public:
		// Fired with the newly selected / activated node.
		Event<TreeNode*> SelectionChanged;
		Event<TreeNode*> NodeActivated;

		// Visual configuration.
		Property<TreeView, wchar_t> CollapsedGlyph { this, "CollapsedGlyph", L'+', InvalidationKind::Visual };
		Property<TreeView, wchar_t> ExpandedGlyph  { this, "ExpandedGlyph",  L'-', InvalidationKind::Visual };
		Property<TreeView, wchar_t> LeafGlyph      { this, "LeafGlyph",      L' ', InvalidationKind::Visual };
		Property<TreeView, bool> ShowGlyphs        { this, "ShowGlyphs",     true, InvalidationKind::Visual };
		Property<TreeView, std::size_t> IndentSize { this, "IndentSize",     2,    InvalidationKind::Measure };

		TreeView();

		// --- Top-level node management (delegates to the invisible root) ---
		TreeNode* AddNode(std::unique_ptr<TreeNode> node);
		TreeNode* AddNode(std::wstring header, std::wstring tag = L"");
		std::unique_ptr<TreeNode> RemoveNode(TreeNode* node);
		void Clear();

		// Exposed for advanced scenarios (building the root directly, iterating).
		TreeNode* GetRoot();
		std::size_t NodeCount() const;

		// --- Selection ---
		TreeNode* GetSelected() const;
		// Selects a node, expanding its ancestors if needed to reveal it.
		bool Select(TreeNode* node);
		void SelectFirst();
		void SelectLast();

		// --- Whole-tree expansion ---
		void ExpandAll();
		void CollapseAll();

		bool IsFocusable() const override { return true; }
		bool OnKeyDown(InputEvent input) override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}
