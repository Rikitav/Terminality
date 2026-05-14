export module terminality:MessageBox;

import <string>;

export namespace terminality
{
	enum class MessageBoxButton
	{
		Ok,
		OkCancel,
		YesNo,
		YesNoCancel
	};

	enum class MessageBoxResult
	{
		None,
		Ok,
		Cancel,
		Yes,
		No
	};

	class MessageBox
	{
	public:
		static MessageBoxResult Show(const std::wstring& title, const std::wstring& message, MessageBoxButton buttons = MessageBoxButton::Ok);
	};
}