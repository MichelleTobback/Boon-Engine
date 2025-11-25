#include "Physics/PhysicsWorld2D.h"
#include "Physics/Physics2D.h"

#include "Scene/GameObject.h"

#include "Component/Rigidbody2D.h"
#include "Component/BoxCollider2D.h"

#include "Core/Time.h"

#include <iostream>

using namespace Boon;

Boon::PhysicsWorld2D::~PhysicsWorld2D()
{
	if (b2World_IsValid(m_PhysicsWorldId))
	{
		b2DestroyWorld(m_PhysicsWorldId);
		m_PhysicsWorldId = b2_nullWorldId;
	}
}

void Boon::PhysicsWorld2D::Begin(Scene* pScene)
{
	if (b2World_IsValid(m_PhysicsWorldId))
		return;

	b2WorldDef worldDef = b2DefaultWorldDef();
	worldDef.gravity = { 0.0f, -9.8f };
	m_PhysicsWorldId = b2CreateWorld(&worldDef);

	auto view = pScene->GetAllGameObjectsWith<Rigidbody2D>();
	for (auto e : view)
	{
		GameObject gameObject( e, pScene);
		auto& transform = gameObject.GetComponent<TransformComponent>();
		auto& rb2d = gameObject.GetComponent<Rigidbody2D>();

		b2BodyDef bodyDef = b2DefaultBodyDef();
		bodyDef.type = Rigidbody2DTypeToBox2DBody((Rigidbody2D::BodyType)rb2d.Type);
		bodyDef.position = { transform.GetLocalPosition().x, transform.GetLocalPosition().y };
		bodyDef.rotation = b2MakeRot(glm::radians(transform.GetWorldRotation().z));
		bodyDef.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(e));
		bodyDef.gravityScale = rb2d.GravityScale;
		rb2d.RuntimeBody = b2CreateBody(m_PhysicsWorldId, &bodyDef);

		if (gameObject.HasComponent<BoxCollider2D>())
		{
			auto& bc2d = gameObject.GetComponent<BoxCollider2D>();

			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = bc2d.Density;
			shapeDef.isSensor = bc2d.IsTrigger;
			shapeDef.enableSensorEvents = true;

			// Set surface material properties
			shapeDef.material.friction = bc2d.Friction;
			shapeDef.material.restitution = bc2d.Restitution;

			// --- Create the polygon (Box2D uses half extents)
			glm::vec3 scale = transform.GetWorldScale();
			b2Polygon box = b2MakeBox(
				bc2d.Size.x * scale.x * 0.5f,
				bc2d.Size.y * scale.y * 0.5f
			);

			// --- Attach the shape
			b2CreatePolygonShape(rb2d.RuntimeBody, &shapeDef, &box);
		}
	}
}

void Boon::PhysicsWorld2D::End(Scene* pScene)
{
	if (b2World_IsValid(m_PhysicsWorldId))
	{
		b2DestroyWorld(m_PhysicsWorldId);
		m_PhysicsWorldId = b2_nullWorldId;
	}
}

void Boon::PhysicsWorld2D::Step(Scene* pScene)
{
	if (!b2World_IsValid(m_PhysicsWorldId))
		return;

	pScene->ForeachGameObjectWith<Rigidbody2D, TransformComponent>([](GameObject e)
		{
			Rigidbody2D& rb = e.GetComponent<Rigidbody2D>();
			auto& transform = e.GetTransform();
			if (!b2Body_IsValid(rb.RuntimeBody))
				return;

			if ((Rigidbody2D::BodyType)rb.Type == Rigidbody2D::BodyType::Kinematic)
			{
				// For kinematic: push Transform -> Box2D
				glm::vec3 pos = transform.GetWorldPosition();
				glm::vec3 rot = transform.GetWorldEulerRotation();

				b2Body_SetTransform(rb.RuntimeBody, { pos.x, pos.y }, b2MakeRot(glm::radians(rot.z)));
			}
			else
			{
				// For dynamic/static: pull from Box2D
				if (rb.FixedRotation)
				{
					b2Transform t = b2Body_GetTransform(rb.RuntimeBody);
					float rot = glm::radians(transform.GetWorldRotation().z);
					b2Body_SetTransform(rb.RuntimeBody, t.p, b2MakeRot(rot));
				}
			}
		});

	const float timeStep = Time::Get().GetFixedTimeStep();
	const int32_t iterations = 3;

	b2World_Step(m_PhysicsWorldId, timeStep, iterations);

	pScene->ForeachGameObjectWith<Rigidbody2D, TransformComponent>([](GameObject e)
		{
			Rigidbody2D& rb = e.GetComponent<Rigidbody2D>();
			auto& transform = e.GetTransform();

			if ((Rigidbody2D::BodyType)rb.Type != Rigidbody2D::BodyType::Dynamic)
				return;

			b2Transform t = b2Body_GetTransform(rb.RuntimeBody);

			glm::vec3 pos3 = transform.GetWorldPosition();
			pos3.x = t.p.x;
			pos3.y = t.p.y;

			transform.SetLocalPosition(pos3);
			if (!rb.FixedRotation)
				transform.SetLocalRotation({ 0.f, 0.f, glm::degrees(b2Rot_GetAngle(t.q)) });
		});

	HandleEvents(pScene);
}

void Boon::PhysicsWorld2D::Update(Scene* pScene)
{
	if (!b2World_IsValid(m_PhysicsWorldId))
		return;
}

bool Boon::PhysicsWorld2D::Raycast(const Ray2D& ray, HitResult2D& result) const
{
	b2RayCastInput b2Ray;
	b2Ray.origin = { ray.Origin.x, ray.Origin.y };
	b2Ray.translation = { ray.Origin.x + ray.Direction.x * ray.Distance, ray.Origin.y + ray.Direction.y * ray.Distance };
	b2Ray.maxFraction = 1.0f;

	b2QueryFilter filter{b2DefaultQueryFilter()};

	b2RayResult out = b2World_CastRayClosest(m_PhysicsWorldId, b2Ray.origin, b2Ray.translation, filter);
	if (out.hit)
	{
		result.point = { out.point.x, out.point.y };
		result.normal = { out.normal.x, out.normal.y };

		b2BodyId body = b2Shape_GetBody(out.shapeId);
		result.gameObject = static_cast<GameObjectID>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(body)));
	}

	return out.hit;
}

void Boon::PhysicsWorld2D::HandleEvents(Scene* pScene)
{
	b2SensorEvents sensorEvents = b2World_GetSensorEvents(m_PhysicsWorldId);

	// Begin overlaps
	for (int i = 0; i < sensorEvents.beginCount; ++i)
	{
		b2SensorBeginTouchEvent ev = sensorEvents.beginEvents[i];

		b2BodyId bodyA = b2Shape_GetBody(ev.sensorShapeId);
		b2BodyId bodyB = b2Shape_GetBody(ev.visitorShapeId);

		GameObject objA = GameObject(static_cast<GameObjectID>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyA))), pScene);
		GameObject objB = GameObject(static_cast<GameObjectID>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyB))), pScene);

		if (objA && objB)
		{
			pScene->OnBeginOverlap(objA, objB);
			pScene->OnBeginOverlap(objB, objA);
		}
	}

	// End overlaps
	for (int i = 0; i < sensorEvents.endCount; ++i)
	{
		b2SensorEndTouchEvent ev = sensorEvents.endEvents[i];

		b2BodyId bodyA = b2Shape_GetBody(ev.sensorShapeId);
		b2BodyId bodyB = b2Shape_GetBody(ev.visitorShapeId);

		GameObject objA = GameObject(static_cast<GameObjectID>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyA))), pScene);
		GameObject objB = GameObject(static_cast<GameObjectID>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyB))), pScene);

		if (objA && objB)
		{
			pScene->OnEndOverlap(objA, objB);
			pScene->OnEndOverlap(objB, objA);
		}
	}
}
