#pragma once
#include "Core/Boon.h"
#include <glm/glm.hpp>

namespace Boon
{
	BCLASS(Name="Box Collider 2D")
	struct BoxCollider2D final
	{
		BPROPERTY()
		glm::vec2 Size = {1.f, 1.f};

		BPROPERTY()
		float Density = 1.f;

		BPROPERTY()
		float Friction = 0.f;

		BPROPERTY()
		float Restitution = 0.f;

		BPROPERTY()
		bool IsTrigger = false;

#ifdef BOON_WITH_EDITOR
		bool DrawDebug{ true };
#endif // BOON_WITH_EDITOR
	};
}