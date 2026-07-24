
#include <terminality/Controls/Canvas.hpp>

using namespace terminality;

Canvas::Canvas()
{
	SetFocusable(false);
}

Size Canvas::MeasureOverride(const Size& availableSize)
{
	int32_t width = availableSize.Width >= 0 ? availableSize.Width : 1;
	int32_t height = availableSize.Height >= 0 ? availableSize.Height : 1;
	return Size(width, height);
}

void Canvas::ArrangeOverride(const Rect& /*contentRect*/)
{
}

void Canvas::RenderOverride(RenderContext& context)
{
	auto now = std::chrono::steady_clock::now();
	float dt = hasLastTime_
		? std::chrono::duration<float>(now - lastTime_).count() : 0.0f;

	lastTime_ = now;
	hasLastTime_ = true;

	if (OnRender)
		OnRender(context, dt);

	// The canvas is never "done": mark itself dirty again so the render tree
	// re-renders it on the next frame (continuous animation).
	InvalidateVisual();
}
