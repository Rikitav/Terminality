#include "ProcService.hpp"

#include "../Common.hpp"

#include <algorithm>
#include <unordered_map>

#include <psapi.h>
#include <tlhelp32.h>

#include "CpuService.hpp"

namespace
{
	std::wstring queryProcessUser(uint32_t pid)
	{
		const HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
		if (process == nullptr)
			return L"-";

		std::wstring result = L"-";
		HANDLE token = nullptr;

		if (OpenProcessToken(process, TOKEN_QUERY, &token))
		{
			DWORD size = 0;
			GetTokenInformation(token, TokenUser, nullptr, 0, &size);
			if (size > 0)
			{
				std::vector<BYTE> buffer(size);
				if (GetTokenInformation(token, TokenUser, buffer.data(), size, &size))
				{
					const auto* tokenUser = reinterpret_cast<TOKEN_USER*>(buffer.data());
					WCHAR name[64] = {};
					WCHAR domain[64] = {};
					DWORD nameLen = 64, domainLen = 64;
					SID_NAME_USE use;
		
					if (LookupAccountSidW(
						nullptr,
						tokenUser->User.Sid,
						name,
						&nameLen,
						domain,
						&domainLen,
						&use))
					{
						if (name[0] != L'\0')
							result = name;
					}
				}
			}

			CloseHandle(token);
		}

		CloseHandle(process);
		return result;
	}

	struct ProcPrev
	{
		uint64_t user = 0;
		uint64_t kernel = 0;
	};

	// State owned by the collector thread.
	std::unordered_map<uint32_t, ProcPrev>& prevProc()
	{
		static std::unordered_map<uint32_t, ProcPrev> value;
		return value;
	}

	std::unordered_map<uint32_t, std::pair<uint64_t, std::wstring>>& userCache()
	{
		static std::unordered_map<uint32_t, std::pair<uint64_t, std::wstring>> value;
		return value;
	}
}

void btop::ProcService::Prime()
{
	std::vector<ProcInfo> dummy;
	Collect(dummy);
}

void btop::ProcService::Collect(std::vector<ProcInfo>& out)
{
	out.clear();

	const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
		return;

	const uint64_t sysTotal = CpuService::LastSysTotalDelta();
	const double cpuCap = 100.0 * std::max<uint32_t>(CpuService::CoreCount(), 1);

	PROCESSENTRY32W entry{ sizeof(entry) };
	for (BOOL ok = Process32FirstW(snapshot, &entry); ok; ok = Process32NextW(snapshot, &entry))
	{
		const uint32_t pid = entry.th32ProcessID;
		if (pid == 0)
			continue;

		ProcInfo proc;
		proc.pid = pid;
		proc.name = entry.szExeFile;
		if (proc.name.size() > 4 && proc.name.compare(proc.name.size() - 4, 4, L".exe") == 0)
			proc.name.resize(proc.name.size() - 4);

		proc.threads = entry.cntThreads;

		const HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
		uint64_t createTime = 0;
		
		if (process != nullptr)
		{
			FILETIME createFt{}, exitFt{}, kernelFt{}, userFt{};
			if (GetProcessTimes(process, &createFt, &exitFt, &kernelFt, &userFt))
			{
				const uint64_t user = (static_cast<uint64_t>(userFt.dwHighDateTime) << 32) | userFt.dwLowDateTime;
				const uint64_t kernel = (static_cast<uint64_t>(kernelFt.dwHighDateTime) << 32) | kernelFt.dwLowDateTime;
				createTime = (static_cast<uint64_t>(createFt.dwHighDateTime) << 32) | createFt.dwLowDateTime;

				const auto it = prevProc().find(pid);
				if (it != prevProc().end()
					&& sysTotal > 0
					&& user >= it->second.user
					&& kernel >= it->second.kernel)
				{
					const uint64_t procDelta = (user - it->second.user) + (kernel - it->second.kernel);
					proc.cpu = std::clamp(procDelta * 100.0 / sysTotal, 0.0, cpuCap);
				}
		
				prevProc()[pid] = ProcPrev{ user, kernel };
			}

			PROCESS_MEMORY_COUNTERS pmc{ sizeof(pmc) };
			if (GetProcessMemoryInfo(process, &pmc, sizeof(pmc)))
				proc.mem = pmc.WorkingSetSize;

			CloseHandle(process);
		}

		proc.user = UserFor(pid, createTime);
		out.push_back(std::move(proc));
	}

	CloseHandle(snapshot);
}

std::wstring btop::ProcService::UserFor(uint32_t pid, uint64_t createTime)
{
	const auto it = userCache().find(pid);
	if (it != userCache().end() && it->second.first == createTime)
		return it->second.second;

	std::wstring user = queryProcessUser(pid);
	userCache()[pid] = { createTime, user };
	return user;
}
