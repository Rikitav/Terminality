module terminality;

import std;

using namespace terminality;

FocusManager& FocusManager::Current()
{
	if (HostApplication::IsUiThread())
		throw std::runtime_error("Cannot get FocusManager within running UI thread or Before UI thread was started.");

	//static FocusManager focusManager;
	//return focusManager;

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

	VisualTreeNode* old = nullptr;
	if (!focusStack.empty())
	{
		old = focusStack.back();
		if (old == node)
			return false;
	}

	VisualTreeNode* parent = node->GetParent();
	if (old == parent)
	{
		focusStack.push_back(node);
	}
	else
	{
		focusStack.clear();
		while (parent != nullptr)
		{
			parent->OnGotFocus();
			parent = parent->GetParent();
		}
	}

	FocusChanged.Emit(old, node);
	node->OnGotFocus();
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
