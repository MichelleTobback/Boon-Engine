#pragma once
#include "Core/Boon.h"
#include "Renderer/Camera.h"

#include "Event/Event.h"

namespace Boon
{
	BCLASS(Name="Camera")
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