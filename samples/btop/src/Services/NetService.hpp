#pragma once

#include <string>

#include "SystemTypes.hpp"

namespace btop
{
	// Network throughput from GetIfTable/GetIfEntry (btop's /sys/class/net/<iface>/statistics equivalent).
	// The 32-bit octet counters are accumulated across wraps.
	class NetService
	{
	public:
		NetService() = delete;

		static void Collect(NetData& out);
		static void NextIface(int direction);
		static void ZeroTotals();
		static std::string CurrentIface();

		static void Prime();
	};
}
