export module terminality:Button;

import std;
import :Geometry;
import :ControlBase;
import :EventSignal;
import :RenderContext;

export namespace terminality
{
	class Button : public ControlBase
	{
		std::wstring text_;

	public:
		EventSignal<> Clicked;

		void SetText(std::wstring text);
		void Click();

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;
	};
}

