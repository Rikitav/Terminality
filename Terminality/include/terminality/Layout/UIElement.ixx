export module terminality:UIElement;

import std;
import :Layout;
import :Geometry;
import :VisualTreeNode;

export namespace terminality
{
	class UIElement : public VisualTreeNode
	{
	protected:
		Size explicitSize_ = Size::Zero;
		Size minSize_ = Size::Auto;
		Size maxSize_ = Size::Auto;

		Thickness margin_;
		HorizontalAlignment horizontalAlignment_ = HorizontalAlignment::Stretch;
		VerticalAlignment verticalAlignment_ = VerticalAlignment::Stretch;

	public:
		// Getters
		Size GetSize() const;
		Size GetMinSize() const;
		Size GetMaxSize() const;

		Size GetActualSize() const;
		Rect GetArrangedRect() const;

		Thickness GetMargin() const;
		HorizontalAlignment GetHorizontalAlignment() const;
		VerticalAlignment GetVerticalAlignment() const;

		// Setters
		virtual void SetSize(Size value) = 0;
		virtual void SetMinSize(Size value) = 0;
		virtual void SetMaxSize(Size value) = 0;

		virtual void SetMargin(Thickness value) = 0;
		virtual void SetHorizontalAlignment(HorizontalAlignment value) = 0;
		virtual void SetVerticalAlignment(VerticalAlignment value) = 0;
	};
}
