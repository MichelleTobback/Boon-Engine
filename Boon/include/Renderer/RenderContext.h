#pragma once
#include "Renderer/RendererTypes.h"

namespace Boon
{
	class Scene;
	class Renderer2D;
	class Renderer3D;
	class UniformBuffer;

	struct RenderContext
	{
		Scene& Scene;
		Renderer2D& Renderer2D;
		Renderer3D& Renderer3D;
		UniformBuffer& ObjectUniformBuffer;
		RenderPhaseID CurrentPhase = 0;
	};
}