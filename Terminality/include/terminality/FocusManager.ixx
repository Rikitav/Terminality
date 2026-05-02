export module terminality:FocusManager;

import std;
import :Focus;
import :InputEvent;
import :VisualTree;
import :EventSignal;

export namespace terminality
{
	class FocusManager
	{
		std::vector<VisualTreeNode*> focusStack;

		FocusManager() = default;

	public:
		EventSignal<VisualTreeNode*, VisualTreeNode*> FocusChanged;

		static FocusManager& Current();

		VisualTreeNode* GetFocused() const;
		bool SetFocused(VisualTreeNode* node);
		bool MoveNext(Direction direction, InputModifier modifiers = InputModifier::None);
	};
}
