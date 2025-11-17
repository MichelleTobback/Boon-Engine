#pragma once
#include "Networking/NetIdentity.h"
#include "Core/Boon.h"
#include <glm/glm.hpp>

namespace Boon
{
	BCLASS(Replicated = "NetTransformSerializer")
	struct NetTransform final
	{
		enum class DirtyFlags : uint32_t
		{
			None = 0,

			PosX = 1 << 1,
			PosY = 1 << 2,
			PosZ = 1 << 3,

			Rot = 1 << 4,

			ScaleX = 1 << 5,
			ScaleY = 1 << 6
		};

		int16_t QPosX, QPosY, QPosZ;
		uint16_t QRotDeg;
		int16_t QScaleX, QScaleY;

		int16_t LastQPosX, LastQPosY, LastQPosZ;
		uint16_t LastQRotDeg;
		int16_t LastQScaleX, LastQScaleY;

		uint32_t DirtyMask = 0;

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
				glm::vec3 scale = transform.GetLocalScale();
				// quantize current transform
				QPosX = QuantizePos(pos.x);
				QPosY = QuantizePos(pos.y);
				QPosZ = QuantizePos(pos.z);
				QRotDeg = QuantizeAngleDeg(transform.GetLocalEulerRotation().z);
				QScaleX = QuantizePos(scale.x);
				QScaleY = QuantizePos(scale.y);

				// compute dirty flags
				DirtyMask = 0;
				if (QPosX != LastQPosX) DirtyMask |= (uint32_t)DirtyFlags::PosX;
				if (QPosY != LastQPosY) DirtyMask |= (uint32_t)DirtyFlags::PosY;
				if (QPosZ != LastQPosZ) DirtyMask |= (uint32_t)DirtyFlags::PosZ;
				if (QRotDeg != LastQRotDeg) DirtyMask |= (uint32_t)DirtyFlags::Rot;
				if (QScaleX != LastQScaleX) DirtyMask |= (uint32_t)DirtyFlags::ScaleX;
				if (QScaleY != LastQScaleY) DirtyMask |= (uint32_t)DirtyFlags::ScaleY;
			}
			else
			{
				Rigidbody2D& rb = gameObject.GetComponent<Rigidbody2D>();
				rb.Type = Rigidbody2D::BodyType::Kinematic;
				glm::vec3 pos = { DequantizePos(QPosX), DequantizePos(QPosY), DequantizePos(QPosZ) };
				float rot = DequantizeAngleDeg(QRotDeg);
				glm::vec3 scale = { DequantizePos(QScaleX), DequantizePos(QScaleY), 1.f };

				transform.SetLocalPosition(glm::mix(transform.GetLocalPosition(), pos, 0.4f));
				transform.SetLocalRotation(0.f, 0.f, LerpAngleDegrees(transform.GetLocalRotation().z, rot, 0.4f));
				transform.SetLocalScale(glm::mix(transform.GetLocalScale(), scale, 0.4f));
			}
		}
	};
}