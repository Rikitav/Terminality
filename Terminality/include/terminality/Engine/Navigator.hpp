#pragma once

#include <memory>

#include <terminality/Framework/VisualTree.hpp>
#include <terminality/Framework/VisualTreeNode.hpp>

namespace terminality
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
