#pragma once

#include <glm/glm.hpp>

namespace Boon
{
	struct QuadVertex
	{
		glm::vec3 Position{};
		glm::vec4 Color{ 1.f, 1.f, 1.f, 1.f };
		int GameObjectID{};
	};
}