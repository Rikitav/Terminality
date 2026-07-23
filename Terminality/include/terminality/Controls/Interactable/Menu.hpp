#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <terminality/Core/Color.hpp>
#include <terminality/Core/Geometry.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Core/Layout.hpp>
#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Framework/Event.hpp>
#include <terminality/Framework/Property.hpp>
#include <terminality/Engine/RenderContext.hpp>

namespace terminality
{
	class Menu;

	struct MenuItem
	{
		std::wstring Text;
		std::wstring ShortcutText;
		std::function<void()> Action;
		std::shared_ptr<Menu> SubMenu;
		bool IsSeparator = false;
		bool IsEnabled = true;
		bool IsCheckable = false;
		bool IsChecked = false;
	};

	class Menu : public ControlBase, public std::enable_shared_from_this<Menu>
	{
		std::vector<MenuItem> items_;
		std::size_t selectedIndex_ = 0;
		std::atomic<bool>* running_ = nullptr;
		Menu* parentMenu_ = nullptr;
		std::function<void()> onSubMenuAction_;

		std::wstring GetDisplayText(const std::wstring& text, wchar_t& accessKey, int32_t& accessKeyIndex) const;
		void OpenSubMenu(Menu* submenu);
		void InvokeItem(MenuItem& item);
		void CloseMenu();
		bool HandleAccessKey(InputKey key);
		void MoveSelection(int delta);
		void SelectNextMatch(wchar_t ch);

	public:
		Property<Menu, Color> DisabledForegroundColor { this, "DisabledForegroundColor", Color::DARK_GRAY, InvalidationKind::Visual };
		Property<Menu, Color> DisabledBackgroundColor { this, "DisabledBackgroundColor", Color::BLACK, InvalidationKind::Visual };

		Menu() = default;

		void AddItem(const std::wstring& text, std::function<void()> action = nullptr);
		void AddItem(const std::string& text, std::function<void()> action = nullptr);
		void AddCheckableItem(const std::wstring& text, bool checked, std::function<void(bool)> action = nullptr);
		void AddSeparator();
		void AddSubMenu(const std::wstring& text, std::shared_ptr<Menu> submenu);
		void Clear();

		bool IsEmpty() const { return items_.empty(); }

		void Open(Point position);

		bool IsFocusable() const override { return true; }
		bool OnKeyDown(InputEvent input) override;
		void OnGotFocus() override;
		void OnLostFocus() override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}
