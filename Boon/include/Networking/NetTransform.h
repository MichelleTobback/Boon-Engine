#pragma once
#include "Networking/NetIdentity.h"
#include "Core/Boon.h"
#include <glm/glm.hpp>

namespace Boon
{
	BCLASS()
	struct NetTransform final
	{
		BPROPERTY(Replicated)
		glm::vec3 Position;

		void LateUpdate(GameObject gameObject)
		{
			NetIdentity& netId = gameObject.GetComponent<NetIdentity>();
			TransformComponent& transform = gameObject.GetTransform();
			if (netId.IsAuthority())
			{
				Position = transform.GetLocalPosition();
			}
			else
			{
				transform.SetLocalPosition(Position);
			}
		}
	};
}