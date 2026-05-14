export module terminality:Navigator;

import <memory>;

import :VisualTree;
import :VisualTreeNode;

export namespace terminality
{
	class Navigator
	{
		Navigator() = default;

	public:
		static Navigator& Current();

		void Navigate(std::unique_ptr<VisualTreeNode> page);
		
		bool CanGoBack() const;
		bool GoBack();
		void GoHome();
	};
}