export module terminality:OpenFileDialog;

import std;

export namespace terminality
{
	class OpenFileDialog
	{
	public:
		static std::optional<std::filesystem::path> Show(const std::wstring& title = L"Open File", const std::filesystem::path& initialDirectory = std::filesystem::current_path());
	};
}