#pragma once
#include "Scene/GameObjectID.h"
#include <glm/glm.hpp>

namespace Boon
{
	struct Ray2D
	{
		glm::vec2 Origin;
		glm::vec2 Direction;
		float Distance;
	};

	struct HitResult2D
	{
		GameObjectID gameObject;
		glm::vec2 point;
		glm::vec2 normal;
	};
}