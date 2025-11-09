#pragma once
#include <box2d/id.h>

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

	private:
		b2WorldId m_PhysicsWorldId = b2_nullWorldId;
	};
}