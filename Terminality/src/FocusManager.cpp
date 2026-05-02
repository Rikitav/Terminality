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
	return focusStack.back();
}

bool FocusManager::SetFocused(VisualTreeNode* node)
{
	if (node == nullptr || !node->IsFocusable())
		return false;

	VisualTreeNode* old = nullptr;
	if (!focusStack.empty())
	{
		if (focusStack.back() == node)
			return false;
		
		old = focusStack.back();
		if (old != nullptr)
			old->OnLostFocus();
	}

	focusStack.push_back(node);
	node->OnGotFocus();
	
	FocusChanged.Emit(old, node);
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

		focusStack.pop_back();
		continue;
	}

	return false;
}
