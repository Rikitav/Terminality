export module terminality:FocusManager;

import std;
import :Focus;
import :VisualTree;
import :EventSignal;

export namespace terminality
{
	class FocusManager
	{
		std::vector<VisualTreeNode*> focusStack;

	public:
		EventSignal<VisualTreeNode*, VisualTreeNode*> FocusChanged;

		static FocusManager& Current();

		VisualTreeNode* GetFocused() const;
		bool SetFocused(VisualTreeNode* node);
		bool MoveNext(NavigationDirection direction);
	};
}
