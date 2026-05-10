export module terminality:FocusManager;

import std;
import :Focus;
import :InputEvent;
import :VisualTreeNode;
import :EventSignal;

export namespace terminality
{
	class FocusManager
	{
		std::vector<VisualTreeNode*> focusStack;

	public:
		FocusManager() = default;

		Event<VisualTreeNode*, VisualTreeNode*> FocusChanged;

		static FocusManager& Current();

		VisualTreeNode* GetFocused() const;
		bool SetFocused(VisualTreeNode* node);
		bool MoveNext(Direction direction, InputModifier modifiers = InputModifier::None);
		void ClearFocus(VisualTreeNode* node);
	};
}
