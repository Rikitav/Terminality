#include "NetService.hpp"

#include "../Common.hpp"

#include <algorithm>
#include <chrono>
#include <mutex>
#include <vector>

#include <iphlpapi.h>
#include <ws2tcpip.h>

namespace
{
	// MIB_IFROW.dwType values (iftable.h constants, defined locally because the
	// Strawberry mingw headers ship an old iphlpapi.h without iftype.h).
	constexpr DWORD IfTypeEthernet = 6;
	constexpr DWORD IfTypeIeee80211 = 71;
	constexpr DWORD IfTypeSoftwareLoopback = 24;

	uint64_t nowMs()
	{
		return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now().time_since_epoch()).count());
	}

	std::wstring ipv4ForIndex(uint32_t index)
	{
		ULONG size = 16 << 10;
		std::vector<BYTE> buffer(size);
		const ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER;
		
		if (NO_ERROR != GetAdaptersAddresses(
			AF_INET,
			flags,
			nullptr,
			reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data()), &size))
		{
			return {};
		}

		for (auto* adapter = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data()); adapter != nullptr; adapter = adapter->Next)
		{
			if (adapter->IfIndex != index && adapter->Ipv6IfIndex != index)
				continue;

			for (auto* unicast = adapter->FirstUnicastAddress; unicast != nullptr; unicast = unicast->Next)
			{
				auto* addr = reinterpret_cast<SOCKADDR_IN*>(unicast->Address.lpSockaddr);
				if (addr->sin_family == AF_INET)
				{
					WCHAR str[16] = {};
					InetNtopW(AF_INET, &addr->sin_addr, str, 16);
					return str;
				}
			}
		}

		return {};
	}

	// State owned by the collector thread.
	std::vector<std::string>& ifaces()
	{
		static std::vector<std::string> value;
		return value;
	}

	std::vector<uint32_t>& ifIndexes()
	{
		static std::vector<uint32_t> value;
		return value;
	}

	int& ifaceIndex()
	{
		static int value = -1;
		return value;
	}

	uint32_t& currentIndex()
	{
		static uint32_t value = 0;
		return value;
	}

	uint64_t& prevDown()
	{
		static uint64_t value = 0;
		return value;
	}

	uint64_t& prevUp()
	{
		static uint64_t value = 0;
		return value;
	}

	uint64_t& prevRawDown()
	{
		static uint64_t value = 0;
		return value;
	}

	uint64_t& prevRawUp()
	{
		static uint64_t value = 0;
		return value;
	}

	uint64_t& downRollover()
	{
		static uint64_t value = 0;
		return value;
	}

	uint64_t& upRollover()
	{
		static uint64_t value = 0;
		return value;
	}

	uint64_t& downOffset()
	{
		static uint64_t value = 0;
		return value;
	}

	uint64_t& upOffset()
	{
		static uint64_t value = 0;
		return value;
	}

	uint64_t& downTop()
	{
		static uint64_t value = 0;
		return value;
	}

	uint64_t& upTop()
	{
		static uint64_t value = 0;
		return value;
	}

	uint64_t& prevTick()
	{
		static uint64_t value = 0;
		return value;
	}

	bool& primed()
	{
		static bool value = false;
		return value;
	}

	std::mutex& stateMutex()
	{
		static std::mutex value;
		return value;
	}

	void selectIface(int index)
	{
		if (ifaces().empty())
			return;

		ifaceIndex() = (index + static_cast<int>(ifaces().size())) % static_cast<int>(ifaces().size());
		currentIndex() = ifIndexes()[ifaceIndex()];
		primed() = false;
	}

	void enumerateIfaces()
	{
		MIB_IFTABLE* table = nullptr;
		ULONG tableSize = 0;
		if (GetIfTable(nullptr, &tableSize, FALSE) == ERROR_INSUFFICIENT_BUFFER)
			table = reinterpret_cast<MIB_IFTABLE*>(malloc(tableSize));

		if (table != nullptr && GetIfTable(table, &tableSize, FALSE) == NO_ERROR)
		{
			for (DWORD i = 0; i < table->dwNumEntries; ++i)
			{
				const MIB_IFROW& row = table->table[i];
				if (row.dwType == IfTypeSoftwareLoopback)
					continue;

				ifaces().emplace_back(reinterpret_cast<const char*>(row.bDescr));
				ifIndexes().push_back(row.dwIndex);
			}
		}

		free(table);

		// Default interface: first ethernet/wifi adapter with traffic.
		int best = -1;
		uint64_t bestTraffic = 0;
		for (size_t i = 0; i < ifIndexes().size(); ++i)
		{
			MIB_IFROW row{};
			row.dwIndex = ifIndexes()[i];
			if (GetIfEntry(&row) != NO_ERROR)
				continue;
		
			if (row.dwType != IfTypeEthernet && row.dwType != IfTypeIeee80211)
				continue;
		
			const uint64_t traffic = row.dwInOctets + row.dwOutOctets;
			if (best == -1 || traffic > bestTraffic)
			{
				best = static_cast<int>(i);
				bestTraffic = traffic;
			}
		}
		
		if (best == -1 && !ifIndexes().empty())
			best = 0;
		
		if (best >= 0)
			selectIface(best);
	}
}

void btop::NetService::Prime()
{
	enumerateIfaces();
	NetData dummy;
	Collect(dummy);
}

void btop::NetService::Collect(NetData& out)
{
	std::lock_guard lock(stateMutex());
	if (ifaceIndex() < 0)
		return;

	MIB_IFROW row{};
	row.dwIndex = currentIndex();
	
	if (GetIfEntry(&row) != NO_ERROR)
		return;

	// dwInOctets/dwOutOctets are 32-bit; accumulate wraps like btop does.
	const uint64_t rawDown = row.dwInOctets;
	const uint64_t rawUp = row.dwOutOctets;
	
	if (primed() && rawDown < prevRawDown())
		downRollover() += prevRawDown();

	if (primed() && rawUp < prevRawUp())
		upRollover() += prevRawUp();
	
	prevRawDown() = rawDown;
	prevRawUp() = rawUp;

	const uint64_t down = downRollover() + rawDown;
	const uint64_t up = upRollover() + rawUp;
	const uint64_t now = nowMs();

	if (primed() && now > prevTick())
	{
		const double seconds = (now - prevTick()) / 1000.0;
		if (down >= prevDown())
			out.downSpeed = static_cast<uint64_t>((down - prevDown()) / seconds);
	
		if (up >= prevUp())
			out.upSpeed = static_cast<uint64_t>((up - prevUp()) / seconds);
	}

	prevDown() = down;
	prevUp() = up;
	prevTick() = now;
	primed() = true;

	out.iface = ifaces()[ifaceIndex()];
	if (out.iface.size() > 20)
		out.iface.resize(20);
	
	out.ipv4 = ipv4ForIndex(currentIndex());
	out.downTotal = down - std::min(down, downOffset());
	out.upTotal = up - std::min(up, upOffset());
	out.downTop = downTop() = std::max(downTop(), out.downSpeed);
	out.upTop = upTop() = std::max(upTop(), out.upSpeed);
}

void btop::NetService::NextIface(int direction)
{
	std::lock_guard lock(stateMutex());
	selectIface(ifaceIndex() + direction);
}

void btop::NetService::ZeroTotals()
{
	std::lock_guard lock(stateMutex());
	downOffset() = prevDown();
	upOffset() = prevUp();
}

std::string btop::NetService::CurrentIface()
{
	if (ifaceIndex() < 0)
		return {};

	return ifaces()[ifaceIndex()];
}
