#include "Physics/PhysicsWorld2D.h"
#include "Physics/Physics2D.h"

#include "Scene/GameObject.h"

#include "Component/Rigidbody2D.h"
#include "Component/BoxCollider2D.h"

#include "Core/Time.h"

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
		bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2d.Type);
		bodyDef.position = { transform.GetLocalPosition().x, transform.GetLocalPosition().y };
		bodyDef.rotation = b2MakeRot(glm::radians(transform.GetWorldRotation().z));
		rb2d.RuntimeBody = b2CreateBody(m_PhysicsWorldId, &bodyDef);

		if (gameObject.HasComponent<BoxCollider2D>())
		{
			auto& bc2d = gameObject.GetComponent<BoxCollider2D>();

			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = bc2d.Density;

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
	{
		const float timeStep = Time::Get().GetFixedTimeStep(); // e.g. 1/60
		const int32_t iterations = 1;

		b2World_Step(m_PhysicsWorldId, timeStep, iterations);

		auto view = pScene->GetAllGameObjectsWith<Rigidbody2D>();
		for (auto e : view)
		{
			GameObject gameObject( e, pScene );
			auto& transform = gameObject.GetComponent<TransformComponent>();
			auto& rb2d = gameObject.GetComponent<Rigidbody2D>();

			b2BodyId bodyId = rb2d.RuntimeBody;
			if (!b2Body_IsValid(bodyId)) continue;

			b2Vec2 pos = b2Body_GetPosition(bodyId);
			float angle = b2Rot_GetAngle(b2Body_GetRotation(bodyId));

			glm::vec3 transformPos = transform.GetWorldPosition();
			transformPos.x = pos.x;
			transformPos.y = pos.y;
			transform.SetLocalPosition(transformPos);
			transform.SetLocalRotation(glm::vec3(0.f, 0.f, glm::degrees(angle)));
		}
	}
}
