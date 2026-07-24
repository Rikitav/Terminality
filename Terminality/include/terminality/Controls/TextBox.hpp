#pragma once

#include <cstdint>
#include <string>

#include <terminality/Core/Geometry.hpp>
#include <terminality/Framework/ControlBase.hpp>
#include <terminality/Framework/Event.hpp>
#include <terminality/Framework/Property.hpp>
#include <terminality/Engine/RenderContext.hpp>
#include <terminality/Core/InputEvent.hpp>
#include <terminality/Core/Layout.hpp>
#include <terminality/Core/Focus.hpp>

namespace terminality
{
	class TextBox : public ControlBase
	{
		std::size_t cursorPosition_ = 0;

	public:
		Property<TextBox, std::wstring> Text	   { this, "Text", L"", InvalidationKind::Measure };
		Property<TextBox, TextWrap> TextWrapping   { this, "TextWrapping", terminality::TextWrap::NoWrap, InvalidationKind::Measure };
		Property<TextBox, TextAlign> TextAlignment { this, "TextAlignment", terminality::TextAlign::Left, InvalidationKind::Visual };
		Property<TextBox, bool> AcceptsReturn	   { this, "AcceptsReturn", false, InvalidationKind::Measure };

		Property<TextBox, wchar_t> PasswordChar		{ this, "PasswordChar", L'\0', InvalidationKind::Visual };
		Property<TextBox, std::wstring> PlaceholderText { this, "PlaceholderText", L"", InvalidationKind::Visual };
		Property<TextBox, Color> PlaceholderForegroundColor { this, "PlaceholderForegroundColor", Color::DARK_GRAY, InvalidationKind::Visual };
		Property<TextBox, Color> PlaceholderBackgroundColor { this, "PlaceholderBackgroundColor", Color::BLACK, InvalidationKind::Visual };
		Property<TextBox, bool> IsReadOnly		{ this, "IsReadOnly", false, InvalidationKind::None };
		Property<TextBox, int32_t> MaxLength	{ this, "MaxLength", -1, InvalidationKind::None };

		Event<> TextChanged;

		TextBox();

		void OnPropertyChanged(const char* propertyName) override;

		bool OnKeyDown(InputEvent input) override;
		bool OnKeyUp(InputEvent input) override;

		bool MoveFocusNext(Direction direction, InputModifier modifiers = InputModifier::None) override;
		void OnGotFocus() override;
		void OnLostFocus() override;

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}
