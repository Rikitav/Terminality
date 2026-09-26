#pragma once

#include "Common.hpp"
#include "DiskList.hpp"
#include "History.hpp"
#include "Services/SystemTypes.hpp"
#include "StatRows.hpp"

// The MEM box: memory rows with sparklines on the left, disk usage on the right.
namespace btop
{
	class MemWidget : public terminality::Border
	{
		History* history_ = nullptr;
		StatRowsControl* rows_ = nullptr;
		DiskListControl* disks_ = nullptr;

	public:
		explicit MemWidget(History* history);

		void Update(const Snapshot& snap);
		void SetGraphSymbol(GraphSymbol symbol);
	};
}
