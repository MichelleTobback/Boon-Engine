#include "Scene/GameObject.h"
#include "Scene/Scene.h"
#include "Component/SceneComponent.h"

using namespace Boon;

Boon::GameObject::GameObject(GameObjectID handle, Scene* pScene)
	: m_Handle{ handle }, m_pScene{ pScene }
{
}

Boon::GameObject::~GameObject()
{
}

void Boon::GameObject::SetActive(bool active)
{
}

bool Boon::GameObject::IsActive() const
{
	return false;
}

void Boon::GameObject::AttachToGameObject(GameObject parent, bool keepWorld)
{
	return GetComponent<SceneComponent>().AttachTo(parent.GetComponent<SceneComponent>(), keepWorld);
}

void Boon::GameObject::DetachGameObject(GameObject child)
{
	return GetComponent<SceneComponent>().Detach(child.GetComponent<SceneComponent>());
}

GameObject Boon::GameObject::GetRoot()
{
	return GetComponent<SceneComponent>().GetRootObject();
}

GameObject Boon::GameObject::GetRoot() const
{
	return GetComponent<SceneComponent>().GetRootObject();
}

bool Boon::GameObject::IsValid() const
{
	return m_pScene && m_pScene->GetRegistry().valid(m_Handle);
}

bool Boon::GameObject::IsRoot() const
{
	return !GetParent().IsValid();
}

void Boon::GameObject::Destroy()
{
	m_pScene->DestroyGameObject(*this);
}

inline GameObject Boon::GameObject::GetParent() const
{
	return GetComponent<SceneComponent>().GetParent();
}

inline const std::vector<GameObject> Boon::GameObject::GetChildren() const
{
	return GetComponent<SceneComponent>().GetChildren();
}

void Boon::GameObject::AttachTo(GameObject parent, bool keepWorld)
{
	GetComponent<SceneComponent>().AttachTo(parent.GetComponent<SceneComponent>(), keepWorld);
}

void Boon::GameObject::Detach(GameObject child)
{
	GetComponent<SceneComponent>().Detach(child.GetComponent<SceneComponent>());
}

void Boon::GameObject::DetachFromParent()
{
	GameObject parent = GetParent();
	if (parent.IsValid())
		parent.Detach(*this);
}

UUID Boon::GameObject::GetUUID() const
{
	return GetComponent<UUIDComponent>().Uuid;
}

TransformComponent& Boon::GameObject::GetTransform()
{
	return GetComponent<TransformComponent>();
}

const TransformComponent& Boon::GameObject::GetTransform() const
{
	return GetComponent<TransformComponent>();
}
