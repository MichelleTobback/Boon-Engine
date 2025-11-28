#pragma once
#include "Physics2DTypes.h"

#include <box2d/id.h>

#include <functional>

struct b2ContactBeginTouchEvent;
namespace Boon
{
	class Scene;
	class PhysicsWorld2D final
	{
	public:
		PhysicsWorld2D() = default;
		~PhysicsWorld2D();

		PhysicsWorld2D(const PhysicsWorld2D& other) = delete;
		PhysicsWorld2D(PhysicsWorld2D&& other) = delete;
		PhysicsWorld2D& operator=(const PhysicsWorld2D& other) = delete;
		PhysicsWorld2D& operator=(PhysicsWorld2D&& other) = delete;

		void Begin(Scene* pScene);
		void End(Scene* pScene);
		void Step(Scene* pScene);
		void Update(Scene* pScene);

		bool Raycast(const Ray2D& ray, HitResult2D& result) const;

	private:
		void SpawnRigidbody(class GameObject* obj);
		void HandleEvents(Scene* pScene);

		b2WorldId m_PhysicsWorldId = b2_nullWorldId;
	};
}