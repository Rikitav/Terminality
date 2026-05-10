export module terminality:ControlBase;

import std;
import :Layout;
import :Geometry;
import :InputEvent;
import :RenderContext;
import :EventSignal;
import :PropertyDescriptor;
import :VisualTreeNode;
import :ContextMenu;

export namespace terminality
{
	class ControlBase;

	typedef std::function<bool(const ControlBase*)> ControlPredicate;
	typedef std::function<void(ControlBase*)> HotkeyCallback;

	class ChildIterator
	{
		const VisualTreeNode* node_;
		std::size_t index_;

	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = VisualTreeNode*;
		using difference_type = std::ptrdiff_t;
		using pointer = VisualTreeNode**;
		using reference = VisualTreeNode*&;

		ChildIterator(const VisualTreeNode* node, std::size_t index)
			: node_(node), index_(index) { }

		VisualTreeNode* operator*() const
		{
			return node_->GetVisualChild(index_);
		}

		ChildIterator& operator++()
		{
			index_++;
			return *this;
		}

		ChildIterator operator++(int)
		{
			ChildIterator temp = *this;
			index_++;
			return temp;
		}

		bool operator==(const ChildIterator& other) const
		{
			return node_ == other.node_ && index_ == other.index_;
		}

		bool operator!=(const ChildIterator& other) const
		{
			return !(*this == other);
		}
	};

	class ControlBase : public VisualTreeNode
	{
		std::unordered_map<InputEvent, HotkeyCallback, InputEventHasher> hotkeys_;

	public:
		Event<const char*> PropertyChanged;
		Event<InputEvent> KeyDown;
		Event<InputEvent> KeyUp;

		Property<ControlBase, Size> MinSize { this, "MinSize", Size::Auto, InvalidationKind::Measure };
		Property<ControlBase, Size> MaxSize { this, "MaxSize", Size::Auto, InvalidationKind::Measure };
		Property<ControlBase, Size> ExpSize { this, "ExpSize", Size::Auto, InvalidationKind::Measure };

		Property<ControlBase, Thickness> Margin                       { this, "Margin", Thickness::Zero, InvalidationKind::Measure };
		Property<ControlBase, HorizaontalAllign> HorizontalAlignment  { this, "HorizontalAlignment", HorizaontalAllign::Stretch, InvalidationKind::Measure };
		Property<ControlBase, VerticalAlign> VerticalAlignment        { this, "VerticalAlignment", VerticalAlign::Stretch, InvalidationKind::Measure };

		Property<ControlBase, Color> ForegroundColor		  { this, "ForegroundColor", Color::WHITE, InvalidationKind::Visual };
		Property<ControlBase, Color> BackgroundColor		  { this, "BackgroundColor", Color::BLACK, InvalidationKind::Visual };
		Property<ControlBase, Color> FocusedForegroundColor { this, "FocusedForegroundColor", Color::BLACK, InvalidationKind::Visual };
		Property<ControlBase, Color> FocusedBackgroundColor { this, "FocusedBackgroundColor", Color::WHITE, InvalidationKind::Visual };

		Property<ControlBase, bool> IsVisible                       { this, "IsVisible", true, InvalidationKind::Visual };
		Property<ControlBase, std::unique_ptr<ContextMenu>> CtxMenu { this, "ContextMenu", nullptr, InvalidationKind::None };

		// Setters
		void SetParent(VisualTreeNode* parent) override;
		void SetLayer(UILayer* layer) override;

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

		// User input
		bool OnKeyDown(InputEvent input) override;
		bool OnKeyUp(InputEvent input) override;
		void OnHotkey(InputModifier modifier, InputKey key, HotkeyCallback callback);

		// Ownership
		void OpenContextMenu();

		virtual std::size_t VisualChildrenCount() const override;
		virtual VisualTreeNode* GetVisualChild(std::size_t index) const override;

		const ChildIterator child_begin() const;
		const ChildIterator child_end() const;

		// Navigation
		void Close();

	protected:
		// Layout
		virtual Size MeasureOverride(const Size& availableSize) = 0;
		virtual void ArrangeOverride(const Rect& finalRect) = 0;
		virtual void RenderOverride(RenderContext& context) = 0;
	};

	template <typename T>
	std::unique_ptr<T> init(std::function<void(T*)> init)
	{
		std::unique_ptr<T> widget = std::make_unique<T>();
		if (init != nullptr)
			init(widget.get());
		
		return std::move(widget);
	}

	template <typename T>
	std::unique_ptr<T> init()
	{
		std::unique_ptr<T> widget = std::make_unique<T>();
		return std::move(widget);
	}
}
