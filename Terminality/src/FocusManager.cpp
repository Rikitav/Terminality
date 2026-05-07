module terminality;

import std;

using namespace terminality;

FocusManager& FocusManager::Current()
{
	if (HostApplication::IsUiThread())
		throw std::runtime_error("Cannot get FocusManager within running UI thread or Before UI thread was started.");

	static FocusManager focusManager;
	return focusManager;
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
		VisualTreeNode* node = *i;
		if (node->MoveFocusNext(direction, modifiers))
			return true;

		node->OnLostFocus();
		focusStack.pop_back();
		continue;
	}

	return false;
}
