#pragma once

#include <optional>
#include <string>

#include <terminality/Core/Layout.hpp>
#include <terminality/Core/Color.hpp>
#include <terminality/Core/Geometry.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Framework/Event.hpp>
#include <terminality/Framework/Property.hpp>
#include <terminality/Framework/Property.hpp>
#include <terminality/Engine/RenderContext.hpp>

namespace terminality
{
	class CheckBox : public ControlBase
	{
		bool isPressed_ = false;

	public:
		Property<CheckBox, std::wstring> Text{ this, "Text", L"", InvalidationKind::Measure };
		Property<CheckBox, Color> PressedForegroundColor{ this, "PressedForegroundColor", Color::BLACK, InvalidationKind::Visual };
		Property<CheckBox, Color> PressedBackgroundColor{ this, "PressedBackgroundColor", Color::CYAN, InvalidationKind::Visual };

		Property<CheckBox, std::optional<bool>> IsChecked{ this, "IsChecked", false, InvalidationKind::Visual };
		Property<CheckBox, bool> IsThreeState{ this, "IsThreeState", false, InvalidationKind::Visual };

		Event<std::optional<bool>> Toggled;
		Event<> Checked;
		Event<> Unchecked;

		void Toggle(std::optional<bool> value);

		void OnPropertyChanged(const char* propertyName) override;

		void OnLostFocus() override;

		bool OnKeyDown(InputEvent input) override;
		bool OnKeyUp(InputEvent input) override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}
