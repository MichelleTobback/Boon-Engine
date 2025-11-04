#include "Component/TransformComponent.h"
#include "Component/SceneComponent.h"
#include "Scene/GameObject.h"
#include "Core/BitFlag.h"

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>
#include <algorithm>

using namespace Boon;

TransformComponent::TransformComponent(SceneComponent* owner)
    : m_Owner{ owner }
{
    SetDirty(TransformFlag::All, true);
}

// -----------------------------------------------------------------------------
// World getters
// -----------------------------------------------------------------------------
const glm::mat4& TransformComponent::GetWorld()
{
    if (IsDirty(TransformFlag::World))
        RecalculateWorldTransform();
    return m_WorldTransform;
}

const glm::vec3& TransformComponent::GetWorldPosition()
{
    if (IsDirty(TransformFlag::Position))
        RecalculateWorldPosition();
    return m_WorldPosition;
}

const glm::quat& TransformComponent::GetWorldRotation()
{
    if (IsDirty(TransformFlag::Rotation))
        RecalculateWorldRotation();
    return m_WorldRotQ;
}

const glm::vec3& TransformComponent::GetWorldEulerRotation()
{
    if (IsDirty(TransformFlag::Rotation))
        RecalculateWorldRotation();
    return m_WorldEuler;
}

const glm::vec3& TransformComponent::GetWorldScale()
{
    if (IsDirty(TransformFlag::Scale))
        RecalculateWorldScale();
    return m_WorldScale;
}

// -----------------------------------------------------------------------------
// Local getters
// -----------------------------------------------------------------------------
const glm::vec3& TransformComponent::GetLocalPosition() const { return m_LocalPosition; }
const glm::quat& TransformComponent::GetLocalRotation() const { return m_LocalRotQ; }
const glm::vec3& TransformComponent::GetLocalEulerRotation() const { return m_LocalEuler; }
const glm::vec3& TransformComponent::GetLocalScale() const { return m_LocalScale; }

// -----------------------------------------------------------------------------
// Setters
// -----------------------------------------------------------------------------
void TransformComponent::SetLocalPosition(float x, float y, float z)
{
    SetLocalPosition({ x, y, z });
}

void TransformComponent::SetLocalPosition(const glm::vec3& position)
{
    if (m_LocalPosition != position)
    {
        m_LocalPosition = position;
        SetDirty(TransformFlag::All, true);
    }
}

void TransformComponent::SetLocalRotation(float x, float y, float z)
{
    SetLocalRotation({ x, y, z });
}

void TransformComponent::SetLocalRotation(const glm::vec3& rotation)
{
    m_LocalEuler = rotation;
    m_LocalRotQ = glm::quat(glm::radians(rotation));
    SetDirty(TransformFlag::All, true);
    SetDirty(TransformFlag::Scale, false);
}

void TransformComponent::SetLocalRotation(const glm::quat& rotation)
{
    m_LocalRotQ = rotation;
    glm::vec3 originalEuler = m_LocalEuler;
    m_LocalEuler = glm::degrees(glm::eulerAngles(rotation));

    // Handle 180° Euler gimbal flips
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

void TransformComponent::SetLocalScale(float x, float y, float z)
{
    SetLocalScale({ x, y, z });
}

void TransformComponent::SetLocalScale(const glm::vec3& scale)
{
    if (m_LocalScale != scale)
    {
        m_LocalScale = scale;
        SetDirty(TransformFlag::All, true);
    }
}

// -----------------------------------------------------------------------------
// Modifiers
// -----------------------------------------------------------------------------
void TransformComponent::Translate(float x, float y, float z)
{
    Translate({ x, y, z });
}

void TransformComponent::Translate(const glm::vec3& translation)
{
    m_LocalPosition += translation;
    SetDirty(TransformFlag::All, true);
}

void TransformComponent::Rotate(float x, float y, float z)
{
    Rotate({ x, y, z });
}

void TransformComponent::Rotate(const glm::vec3& rotation)
{
    SetLocalRotation(GetLocalEulerRotation() + rotation);
}

void TransformComponent::Rotate(const glm::quat& rotation)
{
    SetLocalRotation(GetLocalRotation() * rotation);
}

void TransformComponent::Scale(float x, float y, float z)
{
    Scale({ x, y, z });
}

void TransformComponent::Scale(const glm::vec3& scale)
{
    m_LocalScale += scale;
    SetDirty(TransformFlag::All, true);
}

void TransformComponent::RotateTowards(const glm::vec3 direction)
{
    SetLocalRotation(glm::quatLookAt(glm::normalize(direction), glm::vec3(0.f, 1.f, 0.f)));
}

void TransformComponent::RotateToPoint(const glm::vec3 point)
{
    glm::vec3 direction = point - GetLocalPosition();
    RotateTowards(direction);
}

// -----------------------------------------------------------------------------
// Direction vectors
// -----------------------------------------------------------------------------
const glm::vec3& TransformComponent::GetForward()
{
    if (IsDirty(TransformFlag::Forward))
        RecalculateForward();
    return m_Forward;
}

const glm::vec3& TransformComponent::GetUp()
{
    if (IsDirty(TransformFlag::Up))
        RecalculateUp();
    return m_Up;
}

const glm::vec3& TransformComponent::GetRight()
{
    if (IsDirty(TransformFlag::Right))
        RecalculateRight();
    return m_Right;
}

// -----------------------------------------------------------------------------
// Recalculation
// -----------------------------------------------------------------------------
void TransformComponent::RecalculateWorldTransform()
{
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), m_LocalPosition);
    glm::mat4 rotation = glm::toMat4(m_LocalRotQ);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), m_LocalScale);
    glm::mat4 localMatrix = translation * rotation * scale;

    if (m_Owner && !m_Owner->IsRoot())
        m_WorldTransform = m_Owner->GetParent().GetTransform().GetWorld() * localMatrix;
    else
        m_WorldTransform = localMatrix;

    SetDirty(TransformFlag::World, false);
    SetDirty(TransformFlag::Position, true);
    SetDirty(TransformFlag::Rotation, true);
    SetDirty(TransformFlag::Scale, true);
    SetDirty(TransformFlag::Forward, true);
    SetDirty(TransformFlag::Up, true);
    SetDirty(TransformFlag::Right, true);
}

void TransformComponent::RecalculateWorldPosition()
{
    m_WorldPosition = glm::vec3(GetWorld()[3]);
    SetDirty(TransformFlag::Position, false);
}

void TransformComponent::RecalculateWorldRotation()
{
    glm::mat4 world = GetWorld();
    glm::vec3 skew, scale, translation;
    glm::quat rotation;
    glm::vec4 perspective;
    glm::decompose(world, scale, rotation, translation, skew, perspective);

    m_WorldRotQ = rotation;
    m_WorldEuler = glm::degrees(glm::eulerAngles(rotation));
    SetDirty(TransformFlag::Rotation, false);
}

void TransformComponent::RecalculateWorldScale()
{
    const glm::mat4& world = GetWorld();
    m_WorldScale.x = glm::length(glm::vec3(world[0]));
    m_WorldScale.y = glm::length(glm::vec3(world[1]));
    m_WorldScale.z = glm::length(glm::vec3(world[2]));
    SetDirty(TransformFlag::Scale, false);
}

void TransformComponent::RecalculateForward()
{
    m_Forward = glm::normalize(glm::rotate(GetWorldRotation(), glm::vec3(0.f, 0.f, -1.f)));
    SetDirty(TransformFlag::Forward, false);
}

void TransformComponent::RecalculateUp()
{
    m_Up = glm::normalize(glm::rotate(GetWorldRotation(), glm::vec3(0.f, 1.f, 0.f)));
    SetDirty(TransformFlag::Up, false);
}

void TransformComponent::RecalculateRight()
{
    m_Right = glm::normalize(glm::rotate(GetWorldRotation(), glm::vec3(1.f, 0.f, 0.f)));
    SetDirty(TransformFlag::Right, false);
}

// -----------------------------------------------------------------------------
// Dirty flags
// -----------------------------------------------------------------------------
void TransformComponent::SetDirty(TransformFlag flag, bool isDirty)
{
    BitFlag::Set(m_DirtyFlags, flag, isDirty);

    if (isDirty)
        SetChildrenDirty(flag);
}

bool TransformComponent::IsDirty(TransformFlag flag) const
{
    return BitFlag::IsSet(m_DirtyFlags, flag);
}

void TransformComponent::SetChildrenDirty(TransformFlag flag)
{
    if (!m_Owner)
        return;

    for (auto child : m_Owner->GetChildren())
    {
        if (child.IsValid())
            child.GetTransform().SetDirty(flag, true);
    }
}
