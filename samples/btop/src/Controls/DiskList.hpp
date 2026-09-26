#pragma once

#include <vector>

#include "Common.hpp"
#include "Graphing.hpp"
#include "Services/SystemTypes.hpp"
#include "Theme.hpp"

// Disk usage list for the right half of the MEM box.
namespace btop
{
	class DiskListControl : public terminality::ControlBase
	{
	public:
		std::vector<DiskInfo> Disks;

		DiskListControl()
		{
			SetFocusable(false);
		}

	protected:
		terminality::Size MeasureOverride(const terminality::Size& availableSize) override;
		void ArrangeOverride(const terminality::Rect& contentRect) override { }
		void RenderOverride(terminality::RenderContext& context) override;

		std::size_t VisualChildrenCount() const override { return 0; }
		terminality::VisualTreeNode* GetVisualChild(std::size_t) const override { return nullptr; }
	};
}
