#pragma once

#include <memory>
#include <string>

#include <terminality/Core/Geometry.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Engine/RenderContext.hpp>
#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Framework/Event.hpp>
#include <terminality/Framework/Property.hpp>

namespace terminality
{
	class Expander : public ControlBase
	{
	public:
		Property<Expander, std::wstring> Header { this, "Header", L"", InvalidationKind::Visual };
		Property<Expander, bool> Expanded { this, "Expanded", false, InvalidationKind::Measure };
		Property<Expander, std::unique_ptr<ControlBase>> Content { this, "Content", nullptr, InvalidationKind::Measure,
			[this](const std::unique_ptr<ControlBase>& old) { OnContentChanging(old); } };

		Property<Expander, wchar_t> CollapsedGlyph { this, "CollapsedGlyph", L'+', InvalidationKind::Visual };
		Property<Expander, wchar_t> ExpandedGlyph  { this, "ExpandedGlyph",  L'-', InvalidationKind::Visual };

		Event<bool> ExpandedChanged;

		Expander() = default;

		void Expand();
		void Collapse();
		void Toggle();

		bool IsFocusable() const override { return true; }
		bool OnKeyDown(InputEvent input) override;
		void OnGotFocus() override;
		void OnLostFocus() override;

		void OnPropertyChanged(const char* propertyName) override;
		void OnContentChanging(const std::unique_ptr<ControlBase>& oldContent);

		std::size_t VisualChildrenCount() const override;
		VisualTreeNode* GetVisualChild(std::size_t index) const override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}
