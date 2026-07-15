
#include <cstdint>
#include <algorithm>
#include <stdexcept>
#include <stack>

#include <terminality/Terminality.hpp>

using namespace terminality;

FocusManager& FocusManager::Current()
{
	if (!DispatchTimer::Current().CheckAccess())
		throw std::runtime_error("Cannot get FocusManager within running UI thread or Before UI thread was started.");

	return VisualTree::Current().GetFocusManager();
}

VisualTreeNode* FocusManager::GetFocused() const
{
	if (focusStack.empty())
		return nullptr;

	return focusStack.back();
}

bool FocusManager::SetFocused(VisualTreeNode* node)
{
	if (node == nullptr || !node->IsFocusable())
		return false;

	VisualTreeNode* old = focusStack.empty() ? nullptr : focusStack.back();
	if (old == node)
		return false;

	// Build the path from root to the requested node.
	std::vector<VisualTreeNode*> path;
	for (VisualTreeNode* cur = node; cur != nullptr; cur = cur->GetParent())
		path.push_back(cur);
	std::reverse(path.begin(), path.end());

	// Find the common ancestor already present in the focus stack.
	std::size_t common = 0;
	while (common < focusStack.size() && common < path.size() && focusStack[common] == path[common])
		++common;

	// Pop the old focus tail and notify it lost focus.
	for (std::size_t i = focusStack.size(); i > common; --i)
		focusStack[i - 1]->OnLostFocus();
	focusStack.erase(focusStack.begin() + common, focusStack.end());

	// Push the new focus tail. Push before calling OnGotFocus so that
	// containers which auto-focus a child see their parent already focused.
	for (std::size_t i = common; i < path.size(); ++i)
	{
		VisualTreeNode* n = path[i];
		if (!focusStack.empty() && focusStack.back() == n)
			continue;

		focusStack.push_back(n);
		n->OnGotFocus();
	}

	VisualTreeNode* newFocused = focusStack.empty() ? nullptr : focusStack.back();

	// If a container's OnGotFocus delegated focus to a child recursively,
	// that recursive call already emitted FocusChanged. Emit here only when
	// the originally requested node remains the focused one.
	if (newFocused == node && old != newFocused)
		FocusChanged.Emit(old, newFocused);

	return true;
}

bool FocusManager::MoveNext(Direction direction, InputModifier modifiers)
{
	if (focusStack.empty())
		return false;

	std::vector<VisualTreeNode*> focusStackCopy = focusStack;
	for (auto i = focusStackCopy.rbegin(); i != focusStackCopy.rend(); i++)
	{
		// Check if node can move focus
		VisualTreeNode* searchNode = *i;
		if (!searchNode->MoveFocusNext(direction, modifiers))
		{
			// Cannot move, continue
			focusStack.pop_back();
			continue;
		}

		// Can move, disfocussing all popped nodes
		for (auto i = focusStackCopy.rbegin(); i != focusStackCopy.rend(); i++)
		{
			VisualTreeNode* disfocusNode = *i;
			if (disfocusNode == searchNode)
				break; // Cutting edge

			disfocusNode->OnLostFocus();
		}

		// Success
		return true;
	}

	focusStack = focusStackCopy;
	return false;
}

void FocusManager::ClearFocus(VisualTreeNode* node)
{
	if (focusStack.empty() || node == nullptr)
		return;

	auto it = std::find(focusStack.begin(), focusStack.end(), node);
	if (it != focusStack.end())
	{
		VisualTreeNode* oldFocused = focusStack.back();
		for (auto& dropIt = it; dropIt != focusStack.end(); ++dropIt)
			(*dropIt)->OnLostFocus();

		focusStack.erase(it, focusStack.end());
		VisualTreeNode* newFocused = focusStack.empty() ? nullptr : focusStack.back();

		if (oldFocused != newFocused)
			FocusChanged.Emit(oldFocused, newFocused);
	}
}
