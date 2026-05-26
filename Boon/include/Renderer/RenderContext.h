#pragma once

namespace Boon
{
	class Scene;
	class Renderer2D;
	class UniformBuffer;

	struct RenderContext
	{
		Scene& Scene;
		Renderer2D& Renderer2D;

		UniformBuffer& ObjectUniformBuffer;
	};
}