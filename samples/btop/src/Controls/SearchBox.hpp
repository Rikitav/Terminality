#pragma once

#include "Common.hpp"

namespace btop
{
	class SearchBox : public terminality::TextBox
	{
	public:
		terminality::Event<> Submitted;

	protected:
		bool OnKeyDown(terminality::InputEvent input) override;
	};
}
