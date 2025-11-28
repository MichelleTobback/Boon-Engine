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

			Rot = 1 << 4,

			VelX = 1 << 5,
			VelY = 1 << 6
		};

		glm::vec2 Velocity;

		int16_t QPosX, QPosY, QPosZ;
		uint16_t QRotDeg;
		int16_t QVelX, QVelY;

		int16_t LastQPosX, LastQPosY, LastQPosZ;
		uint16_t LastQRotDeg;
		int16_t LastQVelX, LastQVelY;

		uint32_t DirtyMask = 0;

		void Awake(GameObject gameObject)
		{
			
		}

		void LateUpdate(GameObject gameObject)
		{
			if (!gameObject.HasComponent<NetIdentity>())
				return;

			if (!gameObject.HasComponent<Rigidbody2D>())
				return;

			NetIdentity& netId = gameObject.GetComponent<NetIdentity>();
			TransformComponent& transform = gameObject.GetTransform();
			if (netId.IsAuthority())
			{
				auto& id = gameObject.GetComponent<NetIdentity>();
				if (!id.IsAuthority())
					return;

				glm::vec3 pos = transform.GetLocalPosition();
				Velocity = gameObject.GetComponent<Rigidbody2D>().GetVelocity();
				// quantize current transform
				QPosX = QuantizePos(pos.x);
				QPosY = QuantizePos(pos.y);
				QPosZ = QuantizePos(pos.z);
				QRotDeg = QuantizeAngleDeg(transform.GetLocalEulerRotation().z);
				QVelX = QuantizePos(Velocity.x);
				QVelY = QuantizePos(Velocity.y);

				// compute dirty flags
				DirtyMask = 0;
				if (QPosX != LastQPosX) DirtyMask |= (uint32_t)DirtyFlags::PosX;
				if (QPosY != LastQPosY) DirtyMask |= (uint32_t)DirtyFlags::PosY;
				if (QPosZ != LastQPosZ) DirtyMask |= (uint32_t)DirtyFlags::PosZ;
				if (QRotDeg != LastQRotDeg) DirtyMask |= (uint32_t)DirtyFlags::Rot;
				if (QVelX != LastQVelX) DirtyMask |= (uint32_t)DirtyFlags::VelX;
				if (QVelY != LastQVelY) DirtyMask |= (uint32_t)DirtyFlags::VelY;
			}
			else
			{
				Rigidbody2D& rb = gameObject.GetComponent<Rigidbody2D>();
				rb.Type = (int)Rigidbody2D::BodyType::Kinematic;
				glm::vec3 pos = { DequantizePos(QPosX), DequantizePos(QPosY), DequantizePos(QPosZ) };
				float rot = DequantizeAngleDeg(QRotDeg);
				glm::vec2 vel = { DequantizePos(QVelX), DequantizePos(QVelY) };

				transform.SetLocalPosition(glm::mix(transform.GetLocalPosition(), pos, 0.4f));
				transform.SetLocalRotation(0.f, 0.f, LerpAngleDegrees(transform.GetLocalRotation().z, rot, 0.4f));
				Velocity = glm::mix(Velocity, vel, 0.4f);
			}
		}
	};
}