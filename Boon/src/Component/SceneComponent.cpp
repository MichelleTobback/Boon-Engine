#include "Component/SceneComponent.h"
#include "Component/TransformComponent.h"
#include "Scene/GameObject.h"

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

using namespace Boon;

SceneComponent::SceneComponent(GameObject owner, GameObject parent)
    : m_Owner{ owner }, m_Parent{ parent }
{
}

// -----------------------------------------------------------------------------
// AttachTo
// -----------------------------------------------------------------------------
void SceneComponent::AttachTo(SceneComponent& parent, bool keepWorld)
{
    TransformComponent& transform = m_Owner.GetTransform();

    // Detach from previous parent (if any)
    if (m_Parent.IsValid())
    {
        auto& oldParentScene = m_Parent.GetComponent<SceneComponent>();
        oldParentScene.RemoveChild(m_Owner);
    }

    // Reassign parent
    m_Parent = parent.GetOwner();
    parent.AddChild(m_Owner);

    // Keep world transform if requested
    if (keepWorld)
    {
        TransformComponent& parentTransform = m_Parent.GetTransform();

        // Convert world-space position to local-space position
        const glm::mat4 parentWorldInv = glm::inverse(parentTransform.GetWorld());
        glm::vec4 worldPos = glm::vec4(transform.GetWorldPosition(), 1.0f);
        glm::vec4 localPos = parentWorldInv * worldPos;
        transform.SetLocalPosition(glm::vec3(localPos));

        // Convert world rotation to local rotation
        glm::quat localRot = glm::inverse(parentTransform.GetWorldRotation()) * transform.GetWorldRotation();
        transform.SetLocalRotation(localRot);

        // Convert world scale to local scale
        glm::vec3 localScale = transform.GetWorldScale() / parentTransform.GetWorldScale();
        transform.SetLocalScale(localScale);
    }

    transform.SetDirty(TransformComponent::TransformFlag::All, true);
}

// -----------------------------------------------------------------------------
// Detach
// -----------------------------------------------------------------------------
void SceneComponent::Detach(SceneComponent& child)
{
    TransformComponent& childTransform = child.GetOwner().GetTransform();

    // Preserve child's world transform before detaching
    glm::mat4 world = childTransform.GetWorld();

    // Remove parent relationship
    child.m_Parent = GameObject();
    RemoveChild(child.GetOwner());

    // Decompose world transform to position, rotation, scale
    glm::vec3 scale, translation, skew;
    glm::quat rotation;
    glm::vec4 perspective;
    glm::decompose(world, scale, rotation, translation, skew, perspective);

    // Apply world transform as new local transform
    childTransform.SetLocalPosition(translation);
    childTransform.SetLocalRotation(rotation);
    childTransform.SetLocalScale(scale);

    childTransform.SetDirty(TransformComponent::TransformFlag::All, true);
}

// -----------------------------------------------------------------------------
// Child Management
// -----------------------------------------------------------------------------
void SceneComponent::AddChild(GameObject child)
{
    if (std::find(m_Children.begin(), m_Children.end(), child) == m_Children.end())
        m_Children.push_back(child);
}

void SceneComponent::RemoveChild(GameObject child)
{
    auto it = std::find(m_Children.begin(), m_Children.end(), child);
    if (it != m_Children.end())
        m_Children.erase(it);
}

bool SceneComponent::HasChildren() const
{
    return !m_Children.empty();
}

// -----------------------------------------------------------------------------
// Root Queries
// -----------------------------------------------------------------------------
GameObject SceneComponent::GetRootObject()
{
    GameObject current = GetOwner();
    while (!current.IsRoot())
    {
        current = current.GetParent();
    }
    return current;
}

GameObject SceneComponent::GetRootObject() const
{
    GameObject current = GetOwner();
    while (!current.IsRoot())
    {
        current = current.GetParent();
    }
    return current;
}

SceneComponent SceneComponent::GetRoot()
{
    return GetRootObject().GetComponent<SceneComponent>();
}

SceneComponent SceneComponent::GetRoot() const
{
    return GetRootObject().GetComponent<SceneComponent>();
}

// -----------------------------------------------------------------------------
// State Queries
// -----------------------------------------------------------------------------
bool SceneComponent::IsRoot() const
{
    return !m_Parent.IsValid();
}