#pragma once
#include <glm/glm.hpp>

namespace Boon
{
	struct SpriteRendererComponent final
	{
		SpriteRendererComponent() = default;

		glm::vec4 Color;
	};
}