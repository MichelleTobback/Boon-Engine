#include "Scene/Scene.h"
#include "Scene/GameObject.h"
#include "Component/SceneComponent.h"

using namespace Boon;

Boon::Scene::~Scene()
{
}

GameObject Boon::Scene::Instantiate(const glm::vec3& pos)
{
	GameObjectID handle{ m_Registry.create() };
	GameObject instance{ handle, this };
	SceneComponent& sceneComp = instance.AddComponent<SceneComponent>(instance);
	instance.AddComponent<UUIDComponent>();
	instance.AddComponent<TransformComponent>(&sceneComp);
	return instance;
}

GameObject Boon::Scene::Instantiate(UUID uuid, const glm::vec3& pos)
{
	GameObjectID handle{ m_Registry.create() };
	GameObject instance{ handle, this };
	SceneComponent& sceneComp = instance.AddComponent<SceneComponent>(instance);
	instance.AddComponent<UUIDComponent>(uuid);
	instance.AddComponent<TransformComponent>(&sceneComp);
	return instance;
}

void Boon::Scene::Awake()
{
}

void Boon::Scene::Sleep()
{
}

void Boon::Scene::Update()
{
}

void Boon::Scene::FixedUpdate()
{
}

void Boon::Scene::LateUpdate()
{
}

void Boon::Scene::EndUpdate()
{
	while (!m_ObjectsPendingDestroy.empty())
	{
		UUID uuid = m_ObjectsPendingDestroy.front();
		auto it = m_EntityMap.find(uuid);
		if (it != m_EntityMap.end())
		{
			m_EntityMap.erase(uuid);
			m_Registry.destroy(it->second);
		}
		m_ObjectsPendingDestroy.pop();
	}
}

GameObject Boon::Scene::GetGameObject(UUID uuid)
{
	GameObjectID handle = NullGameObject;

	auto it = m_EntityMap.find(uuid);
	if (it != m_EntityMap.end())
		handle = it->second;
	return GameObject(handle, this);
}

void Boon::Scene::DestroyGameObject(GameObject object)
{
	m_ObjectsPendingDestroy.push(object.GetUUID());
}
