#pragma once

#include "Common.hpp"
#include "CpuWidget.hpp"
#include "History.hpp"
#include "MemWidget.hpp"
#include "NetWidget.hpp"
#include "ProcWidget.hpp"

// The grid hosting the four btop boxes (CPU / MEM / NET / PROC) and the
// star-weight relayout that collapses rows and columns of hidden boxes.
namespace btop
{
	class BoxesGrid : public terminality::Grid
	{
		CpuWidget* cpu_ = nullptr;
		MemWidget* mem_ = nullptr;
		NetWidget* net_ = nullptr;
		ProcWidget* proc_ = nullptr;

	public:
		BoxesGrid(History* history);

		CpuWidget* Cpu() const { return cpu_; }
		MemWidget* Mem() const { return mem_; }
		NetWidget* Net() const { return net_; }
		ProcWidget* Proc() const { return proc_; }

		void SetBoxVisible(int index, bool visible);
		bool IsBoxVisible(int index) const;
		void Relayout();
	};
}
