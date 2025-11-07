#pragma once

#include <glm/glm.hpp>

namespace Boon
{
	struct QuadVertex
	{
		glm::vec3 Position{};
		glm::vec4 Color{ 1.f, 1.f, 1.f, 1.f };
		glm::vec2 TexCoord{ 0.f, 0.f };
		float TexIndex{};
		float TilingFactor{};

		int GameObjectID{};
	};

	struct LineVertex
	{
		glm::vec3 Position{};
		glm::vec4 Color{1.f, 1.f, 1.f, 1.f};

	};
}