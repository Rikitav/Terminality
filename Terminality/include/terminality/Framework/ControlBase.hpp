#pragma once

#include <memory>
#include <functional>
#include <cstdint>
#include <string>

#include <terminality/Core/Color.hpp>
#include <terminality/Core/Layout.hpp>
#include <terminality/Core/Geometry.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Framework/Event.hpp>
#include <terminality/Framework/Property.hpp>
#include <terminality/Engine/RenderContext.hpp>
#include <terminality/Framework/VisualTreeNode.hpp>
#include <terminality/Dialogs/ContextMenu.hpp>

namespace terminality
{
	class ControlBase;

	typedef std::function<bool(const ControlBase*)> ControlPredicate;
	typedef std::function<void(ControlBase*)> HotkeyCallback;

	class ChildIterator
	{
		const VisualTreeNode* node_;
		std::size_t index_;
		mutable VisualTreeNode* current_ = nullptr;

		void updateCurrent() const
		{
			if (node_ != nullptr && index_ < node_->VisualChildrenCount())
				current_ = node_->GetVisualChild(index_);
			else
				current_ = nullptr;
		}

	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = VisualTreeNode*;
		using difference_type = std::ptrdiff_t;
		using pointer = VisualTreeNode**;
		using reference = VisualTreeNode*&;

		ChildIterator(const VisualTreeNode* node, std::size_t index)
			: node_(node), index_(index)
		{
			updateCurrent();
		}

		VisualTreeNode*& operator*() const
		{
			return current_;
		}

		VisualTreeNode** operator->() const
		{
			return &current_;
		}

		ChildIterator& operator++()
		{
			++index_;
			updateCurrent();
			return *this;
		}

		ChildIterator operator++(int)
		{
			ChildIterator temp = *this;
			++index_;
			updateCurrent();
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

		Property<ControlBase, std::string> Tag { this, "Tag", "", InvalidationKind::None };

		Property<ControlBase, Size> MinSize { this, "MinSize", Size::Auto, InvalidationKind::Measure };
		Property<ControlBase, Size> MaxSize { this, "MaxSize", Size::Auto, InvalidationKind::Measure };
		Property<ControlBase, Size> ExpSize { this, "ExpSize", Size::Auto, InvalidationKind::Measure };

		Property<ControlBase, Thickness> Margin                    { this, "Margin", Thickness::Zero, InvalidationKind::Measure };
		Property<ControlBase, HorizontalAlign> HorizontalAlignment { this, "HorizontalAlign", HorizontalAlign::Stretch, InvalidationKind::Measure };
		Property<ControlBase, VerticalAlign> VerticalAlignment     { this, "VerticalAlign", VerticalAlign::Stretch, InvalidationKind::Measure };

		Property<ControlBase, Color> ForegroundColor	     { this, "ForegroundColor", Color::WHITE, InvalidationKind::Visual };
		Property<ControlBase, Color> BackgroundColor	     { this, "BackgroundColor", Color::BLACK, InvalidationKind::Visual };
		Property<ControlBase, Color> FocusedForegroundColor  { this, "FocusedForegroundColor", Color::BLACK, InvalidationKind::Visual };
		Property<ControlBase, Color> FocusedBackgroundColor  { this, "FocusedBackgroundColor", Color::WHITE, InvalidationKind::Visual };
		Property<ControlBase, Color> DisabledForegroundColor { this, "DisabledForegroundColor", Color::DARK_GRAY, InvalidationKind::Visual };
		Property<ControlBase, Color> DisabledBackgroundColor { this, "DisabledBackgroundColor", Color::BLACK, InvalidationKind::Visual };

		Property<ControlBase, bool> IsVisible                       { this, "IsVisible", true, InvalidationKind::Visual };
		Property<ControlBase, bool> IsEnabled                       { this, "IsEnabled", true, InvalidationKind::Visual };
		Property<ControlBase, std::unique_ptr<ContextMenu>> CtxMenu { this, "ContextMenu", nullptr, InvalidationKind::None };

		virtual Color GetEffectiveForegroundColor() const;
		virtual Color GetEffectiveBackgroundColor() const;

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

		static void ResetHotkeyExecutionState();

		virtual std::size_t VisualChildrenCount() const override;
		virtual VisualTreeNode* GetVisualChild(std::size_t index) const override;

		const ChildIterator child_begin() const;
		const ChildIterator child_end() const;

		// Navigation
		void Close();

		template<typename T = ControlBase>
		T* QueryByTag(std::string_view tag)
		{
			if (Tag.Get() == tag)
				return dynamic_cast<T*>(this);

			for (std::size_t i = 0; i < VisualChildrenCount(); ++i)
			{
				VisualTreeNode* childNode = GetVisualChild(i);
				if (auto* childControl = dynamic_cast<ControlBase*>(childNode))
				{
					if (auto* result = childControl->QueryByTag<T>(tag))
						return result;
				}
			}

			return nullptr;
		}

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
