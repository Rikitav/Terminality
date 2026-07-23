#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <terminality/Core/Color.hpp>
#include <terminality/Core/Geometry.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Core/Layout.hpp>
#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Framework/Property.hpp>
#include <terminality/Engine/RenderContext.hpp>

namespace terminality
{
	class Menu;

	class MenuBar : public ControlBase
	{
		struct BarItem
		{
			std::wstring Header;
			std::shared_ptr<Menu> Menu;
		};

		std::vector<BarItem> items_;
		std::size_t selectedIndex_ = 0;

		void OpenSelectedMenu();
		bool HandleAccessKey(InputKey key);
		void MoveSelection(int delta);

	public:
		MenuBar() = default;

		void AddMenu(const std::wstring& header, std::shared_ptr<Menu> menu);
		void AddMenu(const std::string& header, std::shared_ptr<Menu> menu);
		void AddMenu(const std::wstring& header, std::unique_ptr<Menu> menu);
		void AddMenu(const std::string& header, std::unique_ptr<Menu> menu);

		bool IsFocusable() const override { return true; }
		bool OnKeyDown(InputEvent input) override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
		void OnGotFocus() override;
		void OnLostFocus() override;
	};
}
