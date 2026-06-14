#pragma once

#include <string>
#include <filesystem>

namespace terminality
{
	class OpenFileDialog
	{
	public:
		static std::optional<std::filesystem::path> Show(const std::wstring& title = L"Open File", const std::filesystem::path& initialDirectory = std::filesystem::current_path());
	};
}
