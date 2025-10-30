#include "Component/TransformComponent.h"
#include "Component/SceneComponent.h"
#include "Scene/GameObject.h"
#include "Core/BitFlag.h"

#include <glm/gtx/matrix_decompose.hpp>
#include "glm/gtc/quaternion.hpp"

Boon::TransformComponent::TransformComponent(SceneComponent* owner)
	: m_Owner{owner}
	, m_LocalRotQ{ glm::quat(1, 0, 0, 0) }
	, m_WorldRotQ{ glm::quat(1, 0, 0, 0) }
{
}

const glm::mat4& Boon::TransformComponent::GetWorld()
{
	if (IsDirty(TransformFlag::World))
	{
		RecalculateWorldTransform();
	}
	return m_WorldTransform;
}

const glm::vec3& Boon::TransformComponent::GetWorldPosition()
{
	if (IsDirty(TransformFlag::Position))
	{
		RecalculateWorldPosition();
	}
	return m_WorldPosition;
}

const glm::vec3& Boon::TransformComponent::GetLocalPosition() const
{
	return m_LocalPosition;
}

const glm::quat& Boon::TransformComponent::GetWorldRotation()
{
	if (IsDirty(TransformFlag::Rotation))
	{
		RecalculateWorldRotation();
	}
	return m_WorldRotQ;
}

const glm::quat& Boon::TransformComponent::GetLocalRotation() const
{
	return m_LocalRotQ;
}

const glm::vec3& Boon::TransformComponent::GetWorldEulerRotation()
{
	if (IsDirty(TransformFlag::Rotation))
	{
		RecalculateWorldRotation();
	}
	return m_WorldEuler;
}

const glm::vec3& Boon::TransformComponent::GetLocalEulerRotation() const
{
	return m_LocalEuler;
}

const glm::vec3& Boon::TransformComponent::GetWorldScale()
{
	if (IsDirty(TransformFlag::Scale))
	{
		RecalculateWorldScale();
	}
	return m_WorldScale;
}

const glm::vec3& Boon::TransformComponent::GetLocalScale() const
{
	return m_LocalScale;
}

void Boon::TransformComponent::SetLocalPosition(float x, float y, float z)
{
	SetLocalPosition({ x, y, z });
}

void Boon::TransformComponent::SetLocalPosition(const glm::vec3& position)
{
	m_LocalPosition = position;
	SetDirty(TransformFlag::All, true);
}

void Boon::TransformComponent::SetLocalRotation(float x, float y, float z)
{
	SetLocalRotation({ x, y, z });
}

void Boon::TransformComponent::SetLocalRotation(const glm::vec3& rotation)
{
	m_LocalEuler = rotation;
	m_LocalRotQ = glm::quat(glm::radians(rotation));
	SetDirty(TransformFlag::All, true);
	SetDirty(TransformFlag::Scale, false);
}

void Boon::TransformComponent::SetLocalRotation(const glm::quat& rotation)
{
	m_LocalRotQ = rotation;
	glm::vec3 originalEuler{ m_LocalEuler };
	m_LocalEuler = glm::degrees(glm::eulerAngles(rotation));

	if ((std::fabs(m_LocalEuler.x - originalEuler.x) == 180.f) &&
		(std::fabs(m_LocalEuler.z - originalEuler.z) == 180.f))
	{
		m_LocalEuler.x = originalEuler.x;
		m_LocalEuler.y = 180.f - m_LocalEuler.y;
		m_LocalEuler.z = originalEuler.z;
	}
	SetDirty(TransformFlag::All, true);
	SetDirty(TransformFlag::Scale, false);
}

void Boon::TransformComponent::SetLocalScale(float x, float y, float z)
{
	SetLocalScale({ x, y, z });
}

void Boon::TransformComponent::SetLocalScale(const glm::vec3& scale)
{
	m_LocalScale = scale;
	SetDirty(TransformFlag::All, true);
}

void Boon::TransformComponent::Translate(float x, float y, float z)
{
	Translate({ x, y, z });
}

void Boon::TransformComponent::Translate(const glm::vec3& translation)
{
	m_LocalPosition += translation;
	SetDirty(TransformFlag::All, true);
}

void Boon::TransformComponent::Rotate(float x, float y, float z)
{
	Rotate({ x, y, z });
}

void Boon::TransformComponent::Rotate(const glm::vec3& rotation)
{
	SetLocalRotation(GetLocalEulerRotation() + rotation);
}

void Boon::TransformComponent::Rotate(const glm::quat& rotation)
{
	SetLocalRotation(GetLocalRotation() * rotation);
}

void Boon::TransformComponent::Scale(float x, float y, float z)
{
	Scale({ x, y, z });
}

void Boon::TransformComponent::Scale(const glm::vec3& scale)
{
	m_LocalScale += scale;
	SetDirty(TransformFlag::All, true);
}

void Boon::TransformComponent::RotateTowards(const glm::vec3 direction)
{
	SetLocalRotation(glm::quatLookAt(glm::normalize(direction), glm::vec3(0.f, 1.f, 0.f)));
}

void Boon::TransformComponent::RotateToPoint(const glm::vec3 point)
{
	glm::vec3 direction{ point - GetLocalPosition() };
	RotateTowards(direction);
}

const glm::vec3& Boon::TransformComponent::GetForward()
{
	if (IsDirty(TransformFlag::Forward))
		RecalculateForward();
	return m_Forward;
}

const glm::vec3& Boon::TransformComponent::GetUp()
{
	if (IsDirty(TransformFlag::Up))
		RecalculateUp();
	return m_Up;
}

const glm::vec3& Boon::TransformComponent::GetRight()
{
	if (IsDirty(TransformFlag::Right))
		RecalculateRight();
	return m_Right;
}

void Boon::TransformComponent::RecalculateWorldPosition()
{
	m_WorldPosition = glm::vec3(GetWorld()[3]);
	SetDirty(TransformFlag::Position, false);
}

void Boon::TransformComponent::RecalculateWorldRotation()
{
	if (m_Owner && !m_Owner->IsRoot())
		m_WorldEuler = m_Owner->GetParent().GetTransform().GetWorldEulerRotation() + GetLocalEulerRotation();
	else
		m_WorldEuler = GetLocalEulerRotation();

	m_WorldRotQ = glm::quat(glm::radians(m_WorldEuler));
	SetDirty(TransformFlag::Rotation, false);
}

void Boon::TransformComponent::RecalculateWorldScale()
{
	const glm::mat4& world{ GetWorld() };
	m_WorldScale.x = glm::length(world[0]);
	m_WorldScale.y = glm::length(world[1]);
	m_WorldScale.z = glm::length(world[2]);
	SetDirty(TransformFlag::Scale, false);
}

void Boon::TransformComponent::RecalculateWorldTransform()
{
	glm::mat4 translation{ glm::translate(glm::mat4(1.0f), m_LocalPosition) };
	glm::mat4 rotation{ glm::toMat4(m_LocalRotQ) };
	glm::mat4 scale{ glm::scale(glm::mat4(1.0f), m_LocalScale) };
	glm::mat4 trs{ translation * rotation * scale };
	m_WorldTransform = !m_Owner || m_Owner->IsRoot() ? trs : m_Owner->GetParent().GetTransform().GetWorld() * trs;
	SetDirty(TransformFlag::World, false);
}

void Boon::TransformComponent::SetDirty(TransformFlag flag, bool isDirty)
{
	BitFlag::Set(m_DirtyFlags, flag, isDirty);
	if (isDirty)
		SetChildrenDirty(flag);
}

bool Boon::TransformComponent::IsDirty(TransformFlag flag) const
{
	return BitFlag::IsSet(m_DirtyFlags, flag);
}

void Boon::TransformComponent::SetChildrenDirty(TransformFlag flag)
{
	if (!m_Owner)
		return;

	for (auto child : m_Owner->GetChildren())
	{
		child.GetTransform().SetDirty(flag, true);
	}
}

void Boon::TransformComponent::RecalculateForward()
{
	m_Forward = glm::normalize(glm::rotate(GetWorldRotation(), glm::vec3(0.f, 0.f, -1.f)));
	SetDirty(TransformFlag::Forward, false);
}

void Boon::TransformComponent::RecalculateUp()
{
	m_Up = glm::normalize(glm::rotate(GetWorldRotation(), glm::vec3(0.f, 1.f, 0.f)));
	SetDirty(TransformFlag::Up, false);
}

void Boon::TransformComponent::RecalculateRight()
{
	m_Right = glm::normalize(glm::rotate(GetWorldRotation(), glm::vec3(1.f, 0.f, 0.f)));
	SetDirty(TransformFlag::Right, false);
}
