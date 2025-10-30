#include "Component/SceneComponent.h"
#include "Component/TransformComponent.h"

using namespace Boon;

Boon::SceneComponent::SceneComponent(GameObject owner, GameObject parent)
	: m_Owner{owner}, m_Parent{parent}
{
}

void Boon::SceneComponent::AttachTo(SceneComponent& parent, bool keepWorld)
{
	TransformComponent& transform{ m_Owner.GetTransform() };

	if (m_Parent)
		m_Parent.Detach(GetOwner());

	m_Parent = parent.GetOwner();
	if (m_Parent)
	{
		parent.AddChild(GetOwner());

		if (keepWorld)
		{
			TransformComponent& parentTransform{ m_Parent.GetTransform() };
			glm::vec3 localPos{ transform.GetLocalPosition() - parentTransform.GetWorldPosition() };
			transform.SetLocalPosition(localPos);
		}
	}

	transform.SetDirty(TransformComponent::TransformFlag::All, true);
}

void Boon::SceneComponent::Detach(SceneComponent& child)
{
	child.m_Parent = GameObject();
	RemoveChild(child.GetOwner());

	TransformComponent& transform{ GetOwner().GetTransform() };
	TransformComponent& childTransform{ child.GetOwner().GetTransform()};

	childTransform.Translate(transform.GetWorldPosition());
	childTransform.Rotate(transform.GetWorldRotation());
	childTransform.Scale(transform.GetWorldScale());
}

void Boon::SceneComponent::AddChild(GameObject child)
{
	m_Children.push_back(child);
}

void Boon::SceneComponent::RemoveChild(GameObject child)
{
	auto it = std::find(m_Children.begin(), m_Children.end(), child);
	if (it != m_Children.end())
		m_Children.erase(it);
}

GameObject Boon::SceneComponent::GetRootObject()
{
	GameObject current{ GetOwner() };
	while (!current.IsRoot())
	{
		current = current.GetParent();
	}
	return current;
}

GameObject Boon::SceneComponent::GetRootObject() const
{
	GameObject current{ GetOwner() };
	while (!current.IsRoot())
	{
		current = current.GetParent();
	}
	return current;
}

SceneComponent Boon::SceneComponent::GetRoot()
{
	return GetRootObject().GetComponent<SceneComponent>();
}

SceneComponent Boon::SceneComponent::GetRoot() const
{
	return GetRootObject().GetComponent<SceneComponent>();
}

bool Boon::SceneComponent::IsRoot() const
{
	return GetRoot().GetOwner().IsValid();
}

bool Boon::SceneComponent::HasChildren() const
{
	return m_Children.size();
}