#pragma once

#include <string>
#include <memory>

#include <terminality/Core/Color.hpp>
#include <terminality/Core/Geometry.hpp>
#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Framework/Event.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Framework/Property.hpp>
#include <terminality/Engine/RenderContext.hpp>

namespace terminality
{
	class Button : public ControlBase
	{
		bool isPressed_ = false;

		std::wstring displayText_;
		wchar_t accessKey_ = L'\0';
		int32_t accessKeyIndex_ = -1;

		void UpdateAccessKey();

	public:
		Property<Button, std::wstring> Text  { this, "Text", L"", InvalidationKind::Measure };
		Property<Button, Color> PressedForegroundColor { this, "PressedForegroundColor", Color::BLACK, InvalidationKind::Visual };
		Property<Button, Color> PressedBackgroundColor { this, "PressedBackgroundColor", Color::CYAN, InvalidationKind::Visual };
		Property<Button, Color> AccessKeyForegroundColor { this, "AccessKeyForegroundColor", Color::YELLOW, InvalidationKind::Visual };
		Property<Button, Color> AccessKeyBackgroundColor { this, "AccessKeyBackgroundColor", Color::BLACK, InvalidationKind::Visual };

		Property<Button, bool> IsDefault { this, "IsDefault", false, InvalidationKind::Visual };
		Property<Button, bool> IsCancel  { this, "IsCancel",  false, InvalidationKind::Visual };

		Event<> Clicked;

		void Click();

		wchar_t GetAccessKey() const { return accessKey_; }
		const std::wstring& GetDisplayText() const { return displayText_; }

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
