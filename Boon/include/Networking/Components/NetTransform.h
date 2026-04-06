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
			ScaleY = 1 << 6,

			All = 0xFFFFFFFF
		};

		int16_t QPosX, QPosY, QPosZ;
		uint16_t QRotDeg;
		int16_t QScaleX, QScaleY;

		int16_t LastQPosX, LastQPosY, LastQPosZ;
		uint16_t LastQRotDeg;
		int16_t LastQScaleX, LastQScaleY;

		uint32_t DirtyMask = 0;

		DirtyFlags ReplicationFlags = DirtyFlags::All;

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
				QPosX = (uint32_t)ReplicationFlags & (uint32_t)DirtyFlags::PosX ? QuantizePos(pos.x) : QPosX;
				QPosY = (uint32_t)ReplicationFlags & (uint32_t)DirtyFlags::PosY ? QuantizePos(pos.y) : QPosY;
				QPosZ = (uint32_t)ReplicationFlags & (uint32_t)DirtyFlags::PosZ ? QuantizePos(pos.z) : QPosZ;
				QRotDeg = (uint32_t)ReplicationFlags & (uint32_t)DirtyFlags::Rot ? QuantizeAngleDeg(transform.GetLocalEulerRotation().z) : QRotDeg;
				QScaleX = (uint32_t)ReplicationFlags & (uint32_t)DirtyFlags::ScaleX ? QuantizePos(scale.x) : QScaleX;
				QScaleY = (uint32_t)ReplicationFlags & (uint32_t)DirtyFlags::ScaleY ? QuantizePos(scale.y) : QScaleY;

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
				glm::vec3 currentPos = transform.GetLocalPosition();
				glm::vec3 currentScale = transform.GetLocalScale();

				glm::vec3 pos = 
				{ 
					DirtyMask & (uint32_t)DirtyFlags::PosX ? DequantizePos(QPosX) : currentPos.x,
					DirtyMask & (uint32_t)DirtyFlags::PosY ? DequantizePos(QPosY) : currentPos.y,
					DirtyMask & (uint32_t)DirtyFlags::PosZ ? DequantizePos(QPosZ) : currentPos.z
				};
				float rot = DequantizeAngleDeg(QRotDeg);
				glm::vec3 scale = 
				{ 
					DirtyMask & (uint32_t)DirtyFlags::ScaleX ? DequantizePos(QScaleX) : currentScale.x,
					DirtyMask & (uint32_t)DirtyFlags::ScaleY ? DequantizePos(QScaleY) : currentScale.y,
					currentScale.z 
				};

				transform.SetLocalPosition(glm::mix(transform.GetLocalPosition(), pos, 0.4f));
				if (DirtyMask & (uint32_t)DirtyFlags::Rot)
					transform.SetLocalRotation(0.f, 0.f, LerpAngleDegrees(transform.GetLocalRotation().z, rot, 0.4f));
				transform.SetLocalScale(glm::mix(transform.GetLocalScale(), scale, 0.4f));
			}
		}
	};
}