#pragma once

#include <string>
#include <functional>
#include <vector>

#include <terminality/Core/Geometry.hpp>

namespace terminality
{
	struct ContextMenuItem
	{
		std::wstring Text;
		std::function<void()> Action;
	};

	class ContextMenu
	{
		std::vector<ContextMenuItem> items_;

	public:
		ContextMenu() = default;

		void AddItem(const std::wstring& text, std::function<void()> action);
		void Clear();

		void Open(Point position);
	};
}
