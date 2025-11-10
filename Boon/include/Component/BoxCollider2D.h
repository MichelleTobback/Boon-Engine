#pragma once
#include <glm/glm.hpp>

namespace Boon
{
	struct BoxCollider2D final
	{
		glm::vec2 Size = {1.f, 1.f};

		float Density = 1.f;
		float Friction = 0.f;
		float Restitution = 0.f;

		bool IsTrigger = false;

#ifdef BOON_WITH_EDITOR
		bool DrawDebug{ true };
#endif // BOON_WITH_EDITOR
	};
}