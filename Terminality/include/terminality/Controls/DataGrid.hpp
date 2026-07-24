#pragma once

#include <cstdint>
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
	struct DataGridColumn
	{
		std::wstring Header;
		int32_t Width = 10;
	};

	class DataGrid : public ControlBase
	{
		std::vector<DataGridColumn> columns_;
		int32_t scrollOffset_ = 0;
		int32_t firstVisibleRow_ = 0;

	public:
		Property<DataGrid, std::vector<std::vector<std::wstring>>> ItemsSource
			{ this, "ItemsSource", {}, InvalidationKind::Measure };

		Property<DataGrid, int> SelectedIndex { this, "SelectedIndex", -1, InvalidationKind::Visual };

		Property<DataGrid, bool> ShowGridLines { this, "ShowGridLines", true, InvalidationKind::Visual };

		Property<DataGrid, Color> HeaderForegroundColor { this, "HeaderForegroundColor", Color::BLACK, InvalidationKind::Visual };
		Property<DataGrid, Color> HeaderBackgroundColor { this, "HeaderBackgroundColor", Color::WHITE, InvalidationKind::Visual };
		Property<DataGrid, Color> SelectedForegroundColor { this, "SelectedForegroundColor", Color::BLACK, InvalidationKind::Visual };
		Property<DataGrid, Color> SelectedBackgroundColor { this, "SelectedBackgroundColor", Color::WHITE, InvalidationKind::Visual };

		Event<int> SelectionChanged;

		DataGrid() = default;

		void AddColumn(const std::wstring& header, int32_t width);
		void AddColumn(const std::string& header, int32_t width);

		void ClearColumns();

		bool IsFocusable() const override { return true; }

		bool OnKeyDown(InputEvent input) override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;

	private:
		int32_t TotalColumnWidth() const;
		void ScrollToSelection();
	};
}
