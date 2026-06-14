#pragma once

#include <vector>

#include <terminality/Core/Focus.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Framework/VisualTreeNode.hpp>
#include <terminality/Framework/Event.hpp>

namespace terminality
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
