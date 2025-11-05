#include "Scene/Scene.h"
#include "Scene/GameObject.h"
#include "Component/SceneComponent.h"
#include "Component/NameComponent.h"
#include "Component/ECSLifecycle.h"

#include "Component/SpriteAnimatorComponent.h"

using namespace Boon;

Boon::Scene::Scene()
{
	m_pECSlifecycle = std::make_unique<ECSLifecycleSystem>(*this);
}

Boon::Scene::~Scene()
{
}

GameObject Boon::Scene::Instantiate(const glm::vec3& pos)
{
	GameObjectID handle{ m_Registry.create() };
	GameObject instance{ handle, this };
	SceneComponent& sceneComp = instance.AddComponent<SceneComponent>(instance);
	UUIDComponent uuid = instance.AddComponent<UUIDComponent>();
	m_EntityMap[uuid.Uuid] = handle;

	instance.AddComponent<TransformComponent>(&sceneComp).SetLocalPosition(pos);
	instance.AddComponent<NameComponent>("GameObject");
	return instance;
}

GameObject Boon::Scene::Instantiate(UUID uuid, const glm::vec3& pos)
{
	GameObjectID handle{ m_Registry.create() };
	GameObject instance{ handle, this };
	SceneComponent& sceneComp = instance.AddComponent<SceneComponent>(instance);
	instance.AddComponent<UUIDComponent>(uuid);
	instance.AddComponent<TransformComponent>(&sceneComp).SetLocalPosition(pos);
	instance.AddComponent<NameComponent>("GameObject");
	m_EntityMap[uuid] = handle;

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
	m_pECSlifecycle->UpdateAll();
}

void Boon::Scene::FixedUpdate()
{
	m_pECSlifecycle->FixedUpdateAll();
}

void Boon::Scene::LateUpdate()
{
	m_pECSlifecycle->LateUpdateAll();
}

void Boon::Scene::EndUpdate()
{
	while (!m_ObjectsPendingDestroy.empty())
	{
		UUID uuid = m_ObjectsPendingDestroy.front();
		auto it = m_EntityMap.find(uuid);
		if (it != m_EntityMap.end())
		{
			GameObject(it->second, this).DetachFromParent();
			m_Registry.destroy(it->second);
			m_EntityMap.erase(uuid);
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

void Boon::Scene::ForeachGameObject(const std::function<void(GameObject)>& fn)
{
	for (GameObjectID handle : m_Registry.view<GameObjectID>())
	{
		GameObject obj(handle, this);
		fn(obj);
	}
}

void Boon::Scene::DestroyGameObject(GameObject object)
{
	m_ObjectsPendingDestroy.push(object.GetUUID());
}
