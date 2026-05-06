export module terminality:ControlBase;

import std;
import :Geometry;
import :Layout;
import :RenderContext;
import :EventSignal;
import :PropertyDescriptor;
import :VisualTreeNode;

export namespace terminality
{
	class ControlBase : public VisualTreeNode
	{
	public:
		EventSignal<const char*> PropertyChanged;

		PropertyDescriptor<ControlBase, Size> MinSize { this, "MinSize", Size::Auto, InvalidationKind::Measure };
		PropertyDescriptor<ControlBase, Size> MaxSize { this, "MaxSize", Size::Auto, InvalidationKind::Measure };

		PropertyDescriptor<ControlBase, Thickness> Margin                        { this, "Margin", Thickness::Zero, InvalidationKind::Measure };
		PropertyDescriptor<ControlBase, HorizontalAlignment> HorizontalAlignment { this, "HorizontalAlignment", HorizontalAlignment::Stretch, InvalidationKind::Measure };
		PropertyDescriptor<ControlBase, VerticalAlignment> VerticalAlignment     { this, "VerticalAlignment", VerticalAlignment::Stretch, InvalidationKind::Measure };

		PropertyDescriptor<ControlBase, Color> ForegroundColor		  { this, "ForegroundColor", Color::WHITE, InvalidationKind::Visual };
		PropertyDescriptor<ControlBase, Color> BackgroundColor		  { this, "BackgroundColor", Color::BLACK, InvalidationKind::Visual };
		PropertyDescriptor<ControlBase, Color> FocusedForegroundColor { this, "FocusedForegroundColor", Color::BLACK, InvalidationKind::Visual };
		PropertyDescriptor<ControlBase, Color> FocusedBackgroundColor { this, "FocusedBackgroundColor", Color::WHITE, InvalidationKind::Visual };

		PropertyDescriptor<ControlBase, bool> IsVisible { this, "IsVisible", true, InvalidationKind::Visual };

		// Setters
		void SetParent(VisualTreeNode* parent) override;
		void SetFocusable(bool value) override;
		void SetTabStop(bool value) override;
		void SetTabIndex(int value) override;

		// Layout
		Size Measure(const Size& availableSize) override;
		void Arrange(const Rect& finalRect) override;
		void Render(RenderContext& context) override;

		// Depends
		void ApplyInvalidation(InvalidationKind invalidation);
		virtual void OnPropertyChanged(const char* propertyName);

		// Ownership
		virtual const std::span<VisualTreeNode*> GetChildren() const;
	
	//protected:
		Size GetActualSize() const;
		Rect GetArrangedRect() const;
	};

	template <typename T>
	static std::unique_ptr<T> ctor(std::function<void(T*)> init)
	{
		std::unique_ptr<T> widget = std::make_unique<T>();
		init(widget.get());
		return std::move(widget);
	}

	typedef std::function<bool(const ControlBase*)> ControlPredicate;
}
