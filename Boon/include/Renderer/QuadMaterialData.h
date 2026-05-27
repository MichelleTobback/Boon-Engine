#pragma once

#include <glm/glm.hpp>

namespace Boon
{
	struct QuadMaterialData
	{
		glm::vec4 Color{ 1.0f };
		float TilingFactor = 1.0f;

		glm::vec3 Padding{ 0.0f };
	};
}