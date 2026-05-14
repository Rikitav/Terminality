export module terminality:ContextMenu;

import <string>;
import <functional>;

import :Geometry;

export namespace terminality
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