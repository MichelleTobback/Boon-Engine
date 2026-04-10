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
		SpawnRigidbody(&gameObject);
	}
}

void Boon::PhysicsWorld2D::End(Scene*)
{
	m_RigidbodyRuntime = {};

	if (b2World_IsValid(m_PhysicsWorldId))
	{
		b2DestroyWorld(m_PhysicsWorldId);
		m_PhysicsWorldId = b2_nullWorldId;
	}
}

void PhysicsWorld2D::Step(Scene* pScene)
{
	if (!b2World_IsValid(m_PhysicsWorldId))
		return;

	pScene->ForeachGameObjectWith<Rigidbody2D, TransformComponent>(
		[this](GameObject obj)
		{
			auto id = (GameObjectID)obj;
			auto& rb = obj.GetComponent<Rigidbody2D>();
			auto& transform = obj.GetTransform();

			auto* runtime = GetRuntime(id);
			if (!runtime || !b2Body_IsValid(runtime->Body))
			{
				SpawnRigidbody(&obj);
				runtime = GetRuntime(id);
				if (!runtime) return;
			}

			b2BodyId body = runtime->Body;

			if (rb.HasPendingSetTransform)
			{
				b2Body_SetTransform(
					body,
					{ rb.PendingSetPosition.x, rb.PendingSetPosition.y },
					b2MakeRot(glm::radians(rb.PendingSetRotation))
				);
				rb.HasPendingSetTransform = false;
			}

			if (rb.HasPendingSetVelocity)
			{
				b2Body_SetLinearVelocity(body, { rb.PendingSetVelocity.x, rb.PendingSetVelocity.y });
				rb.HasPendingSetVelocity = false;
			}

			if (rb.HasPendingSetAngularVelocity)
			{
				b2Body_SetAngularVelocity(body, rb.PendingSetAngularVelocity);
				rb.HasPendingSetAngularVelocity = false;
			}

			if (rb.PendingForce != glm::vec2(0.0f))
			{
				b2Body_ApplyForceToCenter(body, { rb.PendingForce.x, rb.PendingForce.y }, rb.WakeRequested);
				rb.PendingForce = {};
			}

			if (rb.PendingImpulse != glm::vec2(0.0f))
			{
				b2Body_ApplyLinearImpulseToCenter(body, { rb.PendingImpulse.x, rb.PendingImpulse.y }, rb.WakeRequested);
				rb.PendingImpulse = {};
			}

			if (rb.PendingTorque != 0.0f)
			{
				b2Body_ApplyTorque(body, rb.PendingTorque, rb.WakeRequested);
				rb.PendingTorque = 0.0f;
			}

			rb.WakeRequested = false;

			if ((Rigidbody2D::BodyType)rb.Type == Rigidbody2D::BodyType::Kinematic)
			{
				glm::vec3 pos = transform.GetWorldPosition();
				glm::vec3 rot = transform.GetWorldEulerRotation();

				b2Body_SetTransform(body, { pos.x, pos.y }, b2MakeRot(glm::radians(rot.z)));
			}
		});

	b2World_Step(m_PhysicsWorldId, Time::Get().GetFixedTimeStep(), 3);

	pScene->ForeachGameObjectWith<Rigidbody2D, TransformComponent>(
		[this](GameObject obj)
		{
			auto id = (GameObjectID)obj;
			auto* runtime = GetRuntime(id);
			if (!runtime || !b2Body_IsValid(runtime->Body))
				return;

			auto& rb = obj.GetComponent<Rigidbody2D>();
			auto& transform = obj.GetTransform();

			b2BodyId body = runtime->Body;
			b2Transform t = b2Body_GetTransform(body);

			rb.Position = { t.p.x, t.p.y };
			rb.Rotation = glm::degrees(b2Rot_GetAngle(t.q));

			b2Vec2 vel = b2Body_GetLinearVelocity(body);
			rb.Velocity = { vel.x, vel.y };
			rb.AngularVelocity = b2Body_GetAngularVelocity(body);
			rb.Awake = b2Body_IsAwake(body);

			if ((Rigidbody2D::BodyType)rb.Type == Rigidbody2D::BodyType::Dynamic)
			{
				glm::vec3 pos3 = transform.GetWorldPosition();
				pos3.x = rb.Position.x;
				pos3.y = rb.Position.y;
				transform.SetLocalPosition(pos3);

				if (!rb.FixedRotation)
					transform.SetLocalRotation({ 0.f, 0.f, rb.Rotation });
			}
		});

	HandleEvents(pScene);
}

void Boon::PhysicsWorld2D::Update(Scene*)
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

	if (!b2World_IsValid(m_PhysicsWorldId))
		return false;

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

void PhysicsWorld2D::SpawnRigidbody(GameObject* obj)
{
	auto id = (GameObjectID)*obj;
	auto& transform = obj->GetComponent<TransformComponent>();
	auto& rb = obj->GetComponent<Rigidbody2D>();

	b2BodyDef bodyDef = b2DefaultBodyDef();
	bodyDef.type = Rigidbody2DTypeToBox2DBody((Rigidbody2D::BodyType)rb.Type);
	bodyDef.position = { transform.GetLocalPosition().x, transform.GetLocalPosition().y };
	bodyDef.rotation = b2MakeRot(glm::radians(transform.GetWorldRotation().z));
	bodyDef.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(id));
	bodyDef.gravityScale = rb.GravityScale;
	bodyDef.linearDamping = rb.LinearDamping;
	bodyDef.angularDamping = rb.AngularDamping;

	RigidbodyRuntime runtime;
	runtime.Body = b2CreateBody(m_PhysicsWorldId, &bodyDef);
	runtime.Valid = b2Body_IsValid(runtime.Body);

	b2MassData massData = b2Body_GetMassData(runtime.Body);
	massData.mass = rb.GetMass();
	b2Body_SetMassData(runtime.Body, massData);

	m_RigidbodyRuntime[id] = runtime;

	if (obj->HasComponent<BoxCollider2D>())
	{
		auto& bc2d = obj->GetComponent<BoxCollider2D>();

		b2ShapeDef shapeDef = b2DefaultShapeDef();
		shapeDef.density = bc2d.Density;
		shapeDef.isSensor = bc2d.IsTrigger;
		shapeDef.enableSensorEvents = true;
		shapeDef.material.friction = bc2d.Friction;
		shapeDef.material.restitution = bc2d.Restitution;

		glm::vec3 scale = transform.GetWorldScale();
		b2Polygon box = b2MakeBox(
			bc2d.Size.x * scale.x * 0.5f,
			bc2d.Size.y * scale.y * 0.5f
		);

		b2CreatePolygonShape(runtime.Body, &shapeDef, &box);
	}

	rb.DirtyBodyDef = false;
}

PhysicsWorld2D::RigidbodyRuntime* PhysicsWorld2D::GetRuntime(GameObjectID id)
{
	auto it = m_RigidbodyRuntime.find(id);
	if (it == m_RigidbodyRuntime.end())
		return nullptr;
	return &it->second;
}
