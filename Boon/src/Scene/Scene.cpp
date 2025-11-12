#include "Scene/Scene.h"
#include "Scene/GameObject.h"

#include "Component/SceneComponent.h"
#include "Component/NameComponent.h"
#include "Component/ECSLifecycle.h"

#include "Core/Time.h"

#include "Physics/Physics2D.h"

#include <iostream>

using namespace Boon;

Boon::Scene::Scene(const std::string& name)
	: m_Name{name}
{
	m_pECSlifecycle = std::make_unique<ECSLifecycleSystem>(*this);
}

GameObject Boon::Scene::Instantiate(UUID uuid, GameObjectID id)
{
	GameObjectID handle{ m_Registry.create(id) };
	GameObject instance{ handle, this };

	SceneComponent& sceneComp = instance.AddComponent<SceneComponent>(instance);
	instance.AddComponent<UUIDComponent>(uuid);
	instance.AddComponent<TransformComponent>(&sceneComp).SetLocalPosition({});
	instance.AddComponent<NameComponent>("GameObject");
	m_EntityMap[uuid] = handle;

	return instance;
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
	m_Running = true;

	m_pECSlifecycle->AwakeAll();

	m_Physics2D.Begin(this);
}

void Boon::Scene::Sleep()
{
	m_Physics2D.End(this);
	m_Running = false;
}

void Boon::Scene::Update()
{
	OnUpdate();
	LateUpdate();
}

void Boon::Scene::FixedUpdate()
{
	m_pECSlifecycle->FixedUpdateAll();
	m_Physics2D.Step(this);
}

void Boon::Scene::OnUpdate()
{
	m_Physics2D.Update(this);
	m_pECSlifecycle->UpdateAll();
}

void Boon::Scene::LateUpdate()
{
	m_pECSlifecycle->LateUpdateAll();

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

bool Boon::Scene::Raycast2D(const Ray2D& ray, HitResult2D& result) const
{
	return m_Physics2D.Raycast(ray, result);
}

void Boon::Scene::OnBeginOverlap(GameObject overlapped, GameObject other)
{
	m_pECSlifecycle->BeginOverlap(overlapped, other);
}

void Boon::Scene::OnEndOverlap(GameObject overlapped, GameObject other)
{
	m_pECSlifecycle->EndOverlap(overlapped, other);
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
