#pragma once

#include <chrono>
#include <functional>

#include <terminality/Core/Geometry.hpp>
#include <terminality/Engine/RenderContext.hpp>
#include <terminality/Framework/ControlBase.hpp>

namespace terminality
{
	/// A self-invalidating drawing surface. OnRender is invoked every frame, so
	/// the visual is effectively never validated -- ideal for animations. The
	/// callback receives the render context (scoped to the canvas area) and the
	/// delta time (seconds) since the previous frame.
	class Canvas : public ControlBase
	{
	public:
		std::function<void(RenderContext& context, float dt)> OnRender;

		Canvas();

	protected:
		Size MeasureOverride(const Size& availableSize) override;
		void ArrangeOverride(const Rect& contentRect) override;
		void RenderOverride(RenderContext& context) override;

	private:
		std::chrono::steady_clock::time_point lastTime_;
		bool hasLastTime_ = false;
	};
}
