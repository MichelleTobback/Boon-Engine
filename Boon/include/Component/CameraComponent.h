#pragma once

#include "Renderer/Camera.h"

#include "Event/Event.h"

namespace Boon
{
	struct CameraComponent final
	{
		CameraComponent(bool viewportSize = true);
		CameraComponent(const Camera& camera, bool viewportSize = true);
		~CameraComponent();

		Camera Camera;
		bool Active;
		bool ViewportSize;
	};
}