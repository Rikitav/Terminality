#pragma once

#include "Common.hpp"
#include "Graphing.hpp"
#include "History.hpp"
#include "Services/SystemTypes.hpp"

// The NET box: download/upload history graphs on the left, live stats on the right. Owns the graphs' auto-scaling state.
namespace btop
{
	class NetWidget : public terminality::Border
	{
		History* history_ = nullptr;
		GraphControl* downGraph_ = nullptr;
		GraphControl* upGraph_ = nullptr;
		terminality::Label* ifaceLabel_ = nullptr;
		terminality::Label* downInfo_ = nullptr;
		terminality::Label* upInfo_ = nullptr;

		bool autoScale_ = true;
		int64_t maxDown_ = 10 << 10;
		int64_t maxUp_ = 10 << 10;

		void AdaptScale(const Snapshot& snap);

	public:
		NetWidget(History* history);

		void Update(const Snapshot& snap);
		void ToggleAutoScale();
		void SetGraphSymbol(GraphSymbol symbol);
	};
}
