#pragma once

#include "Common.hpp"
#include "CoreGrid.hpp"
#include "Graphing.hpp"
#include "History.hpp"
#include "Services/SystemTypes.hpp"

namespace btop
{
	class CpuWidget : public terminality::Border
	{
		GraphControl* graphUpper_ = nullptr;
		GraphControl* graphLower_ = nullptr;
		MeterControl* meter_ = nullptr;
		terminality::Label* pctLabel_ = nullptr;
		terminality::Label* freqLabel_ = nullptr;
		terminality::Label* uptimeLabel_ = nullptr;
		CoreGridControl* coreGrid_ = nullptr;

	public:
		CpuWidget(History* history);

		void Update(const Snapshot& snap);
		void SetGraphSymbol(GraphSymbol symbol);
	};
}
