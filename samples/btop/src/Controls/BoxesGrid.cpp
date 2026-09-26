#include "BoxesGrid.hpp"

using namespace terminality;

namespace
{
	constexpr int BoxCpu = 0;
	constexpr int BoxMem = 1;
	constexpr int BoxNet = 2;
	constexpr int BoxProc = 3;
}

btop::BoxesGrid::BoxesGrid(History* history)
{
	HorizontalAlignment = HorizontalAlign::Stretch;
	VerticalAlignment = VerticalAlign::Stretch;
	BackgroundColor = theme::mainBg;

	cpu_ = AddChild(0, 0, 1, 2, std::make_unique<CpuWidget>(history));
	mem_ = AddChild(1, 0, 1, 1, std::make_unique<MemWidget>(history));
	net_ = AddChild(2, 0, 1, 1, std::make_unique<NetWidget>(history));
	proc_ = AddChild(1, 1, 2, 1, std::make_unique<ProcWidget>());

	Relayout();
}

void btop::BoxesGrid::SetBoxVisible(int index, bool visible)
{
	switch (index)
	{
		case BoxCpu: cpu_->IsVisible = visible; break;
		case BoxMem: mem_->IsVisible = visible; break;
		case BoxNet: net_->IsVisible = visible; break;
		case BoxProc: proc_->IsVisible = visible; break;
	}
}

bool btop::BoxesGrid::IsBoxVisible(int index) const
{
	switch (index)
	{
		case BoxCpu: return cpu_->IsVisible.Get();
		case BoxMem: return mem_->IsVisible.Get();
		case BoxNet: return net_->IsVisible.Get();
		case BoxProc: return proc_->IsVisible.Get();
	}

	return false;
}

void btop::BoxesGrid::Relayout()
{
	const bool showCpu = cpu_->IsVisible.Get();
	const bool showMem = mem_->IsVisible.Get();
	const bool showNet = net_->IsVisible.Get();
	const bool showProc = proc_->IsVisible.Get();

	// Bitmask: [CPU: bit 3] [MEM: bit 2] [NET: bit 1] [PROC: bit 0]
	const uint8_t mask =
		(showCpu ? 0b1000 : 0) |
		(showMem ? 0b0100 : 0) |
		(showNet ? 0b0010 : 0) |
		(showProc ? 0b0001 : 0);

	struct LayoutSpec {
		const char* rows;
		const char* cols;
	};

	// Index corresponds directly to the bitmask [0..15]
	static constexpr LayoutSpec kLayouts[16] = {
		// mask     CPU MEM NET PROC   Rows           Cols
		/* 0x0 */ { "0,0,0",           "100*"    }, // 0 0 0 0
		/* 0x1 */ { "0,100*,0",        "0,100*"  }, // 0 0 0 1 (PROC only: mem row gets 100* for span)
		/* 0x2 */ { "0,0,100*",        "100*"    }, // 0 0 1 0 (NET only)
		/* 0x3 */ { "0,0,100*",        "45*,55*" }, // 0 0 1 1 (NET + PROC)
		/* 0x4 */ { "0,100*,0",        "100*"    }, // 0 1 0 0 (MEM only)
		/* 0x5 */ { "0,100*,0",        "45*,55*" }, // 0 1 0 1 (MEM + PROC)
		/* 0x6 */ { "0,40*,28*",       "100*"    }, // 0 1 1 0 (MEM + NET)
		/* 0x7 */ { "0,40*,28*",       "45*,55*" }, // 0 1 1 1 (MEM + NET + PROC)
		/* 0x8 */ { "100*,0,0",        "100*"    }, // 1 0 0 0 (CPU only)
		/* 0x9 */ { "32*,68*,0",       "0,100*"  }, // 1 0 0 1 (CPU + PROC)
		/* 0xA */ { "32*,0,28*",       "100*"    }, // 1 0 1 0 (CPU + NET)
		/* 0xB */ { "32*,0,68*",       "45*,55*" }, // 1 0 1 1 (CPU + NET + PROC)
		/* 0xC */ { "32*,40*,0",       "100*"    }, // 1 1 0 0 (CPU + MEM)
		/* 0xD */ { "32*,40*,0",       "45*,55*" }, // 1 1 0 1 (CPU + MEM + PROC)
		/* 0xE */ { "32*,40*,28*",     "100*"    }, // 1 1 1 0 (CPU + MEM + NET)
		/* 0xF */ { "32*,40*,28*",     "45*,55*" }, // 1 1 1 1 (All visible)
	};

	const auto& layout = kLayouts[mask];
	SetRowDefinitions(layout.rows);
	SetColumnDefinitions(layout.cols);
}
