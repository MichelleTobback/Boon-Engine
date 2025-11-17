#pragma once
#include "Networking/ReplicationUtils.h"
#include "Networking/NetIdentity.h"
#include "Component/Rigidbody2D.h"
#include "Core/Boon.h"
#include <glm/glm.hpp>
#include <glm/common.hpp>

namespace Boon
{
	using namespace ReplicationUtils;

	BCLASS(Replicated="NetRigidbody2DSerializer")
	struct NetRigidbody2D final
	{
		enum class DirtyFlags : uint32_t
		{
			None = 0,

			PosX = 1 << 1,
			PosY = 1 << 2,
			PosZ = 1 << 3,

			Rot = 1 << 4
		};

		int16_t QPosX, QPosY, QPosZ;
		uint16_t QRotDeg;

		int16_t LastQPosX, LastQPosY, LastQPosZ;
		uint16_t LastQRotDeg;

		uint32_t DirtyMask = 0;

		void Awake(GameObject gameObject)
		{
			
		}

		void LateUpdate(GameObject gameObject)
		{
			NetIdentity& netId = gameObject.GetComponent<NetIdentity>();
			TransformComponent& transform = gameObject.GetTransform();
			if (netId.IsAuthority())
			{
				auto& id = gameObject.GetComponent<NetIdentity>();
				if (!id.IsAuthority())
					return;

				glm::vec3 pos = transform.GetLocalPosition();
				// quantize current transform
				QPosX = QuantizePos(pos.x);
				QPosY = QuantizePos(pos.y);
				QPosZ = QuantizePos(pos.z);
				QRotDeg = QuantizeAngleDeg(transform.GetLocalEulerRotation().z);

				// compute dirty flags
				DirtyMask = 0;
				if (QPosX != LastQPosX) DirtyMask |= (uint32_t)DirtyFlags::PosX;
				if (QPosY != LastQPosY) DirtyMask |= (uint32_t)DirtyFlags::PosY;
				if (QPosZ != LastQPosZ) DirtyMask |= (uint32_t)DirtyFlags::PosZ;
				if (QRotDeg != LastQRotDeg) DirtyMask |= (uint32_t)DirtyFlags::Rot;
			}
			else
			{
				Rigidbody2D& rb = gameObject.GetComponent<Rigidbody2D>();
				rb.Type = Rigidbody2D::BodyType::Kinematic;
				glm::vec3 pos = { DequantizePos(QPosX), DequantizePos(QPosY), DequantizePos(QPosZ) };
				float rot = DequantizeAngleDeg(QRotDeg);

				transform.SetLocalPosition(glm::mix(transform.GetLocalPosition(), pos, 0.4f));
				transform.SetLocalRotation(0.f, 0.f, LerpAngleDegrees(transform.GetLocalRotation().z, rot, 0.4f));
			}
		}
	};
}