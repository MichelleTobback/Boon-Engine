#include "Scene/Scene.h"
#include "Scene/GameObject.h"
#include "Component/SceneComponent.h"
#include "Component/NameComponent.h"
#include "Component/ECSLifecycle.h"

#include "Component/SpriteAnimatorComponent.h"
#include "Component/Rigidbody2D.h"

// Box2D
#include "box2d/b2_world.h"
#include "box2d/b2_body.h"
#include "box2d/b2_fixture.h"
#include "box2d/b2_polygon_shape.h"
#include "box2d/b2_circle_shape.h"

using namespace Boon;

Boon::Scene::Scene(const std::string& name)
	: m_Name{name}
{
	m_pECSlifecycle = std::make_unique<ECSLifecycleSystem>(*this);
}

Boon::Scene::~Scene()
{
	delete m_PhysicsWorld;
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
	OnPhysics2DStart();
}

void Boon::Scene::Sleep()
{
	OnPhysics2DStop();
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

	EndUpdate();
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

void Boon::Scene::OnPhysics2DStart()
{

}

void Boon::Scene::OnPhysics2DStop()
{

}

void Boon::Scene::UpdatePhysics2D()
{
	// Physics
	{
		const float timeStep = Time::Get().GetFixedTimeStep(); // e.g. 1/60
		const int32 velocityIterations = 8;
		const int32 positionIterations = 3;

		m_PhysicsWorld->Step(timeStep, velocityIterations, positionIterations);

		// Retrieve transform from Box2D
		auto view = m_Registry.view<Rigidbody2D>();
		for (auto e : view)
		{
			GameObject gameObject = { e, this };
			auto& transform = gameObject.GetComponent<TransformComponent>();
			auto& rb2d = gameObject.GetComponent<Rigidbody2D>();

			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			const auto& position = body->GetPosition();
			transform.Translation.x = position.x;
			transform.Translation.y = position.y;
			transform.Rotation.z = body->GetAngle();
		}
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
