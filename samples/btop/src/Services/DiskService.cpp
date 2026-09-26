#include "DiskService.hpp"

#include "../Common.hpp"

void btop::DiskService::Collect(std::vector<DiskInfo>& out)
{
	out.clear();

	WCHAR drives[512] = {};
	if (GetLogicalDriveStringsW(512, drives) == 0)
		return;

	for (const WCHAR* drive = drives; *drive != L'\0'; drive += wcslen(drive) + 1)
	{
		if (GetDriveTypeW(drive) != DRIVE_FIXED)
			continue;

		ULARGE_INTEGER freeBytes{}, totalBytes{};
		if (!GetDiskFreeSpaceExW(drive, nullptr, &totalBytes, &freeBytes))
			continue;

		DiskInfo disk;
		WCHAR label[MAX_PATH + 1] = {};
		
		if (GetVolumeInformationW(drive, label, MAX_PATH, nullptr, nullptr, nullptr, nullptr, 0) && label[0] != L'\0')
		{
			disk.name = std::wstring(drive, 2) + L" " + label;
		}
		else
		{
			disk.name.assign(drive, 2);
		}

		disk.total = totalBytes.QuadPart;
		disk.free = freeBytes.QuadPart;
		disk.used = disk.total - disk.free;
		out.push_back(std::move(disk));
	}
}
