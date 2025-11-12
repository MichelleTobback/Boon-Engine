#include "Scene/GameObject.h"
#include "Scene/Scene.h"
#include "Component/SceneComponent.h"
#include "Reflection/BClass.h"

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

void* GameObject::AddComponentFromClass(BClass* pClass)
{
	if (!pClass || !pClass->addComponent)
		return nullptr;

	return pClass->addComponent(*this);
}

void* Boon::GameObject::GetComponentByClass(BClass* pClass)
{
	return pClass->getComponent(*this);
}

bool Boon::GameObject::HasComponentByClass(BClass* pClass)
{
	return pClass->hasComponent(*this);
}

void Boon::GameObject::RemoveComponentByClass(BClass* pClass)
{
	pClass->removeComponent(*this);
}

void Boon::GameObject::Destroy()
{
	m_pScene->DestroyGameObject(*this);
}

GameObject Boon::GameObject::GetParent() const
{
	return GetComponent<SceneComponent>().GetParent();
}

const std::vector<GameObject> Boon::GameObject::GetChildren() const
{
	std::vector<GameObject> children;
	for (GameObjectID id : GetComponent<SceneComponent>().GetChildren())
		children.push_back(GameObject(id, m_pScene));
	return children;
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
