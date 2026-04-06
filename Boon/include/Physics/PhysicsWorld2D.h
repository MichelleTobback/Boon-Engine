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
        /**
         * @brief 2D physics world wrapper using Box2D.
         *
         * Manages rigidbodies and physics stepping for a Scene.
         */
        PhysicsWorld2D() = default;

        /**
         * @brief Destroy the PhysicsWorld2D and release physics resources.
         */
        ~PhysicsWorld2D();

		PhysicsWorld2D(const PhysicsWorld2D& other) = delete;
		PhysicsWorld2D(PhysicsWorld2D&& other) = delete;
		PhysicsWorld2D& operator=(const PhysicsWorld2D& other) = delete;
		PhysicsWorld2D& operator=(PhysicsWorld2D&& other) = delete;

        /**
         * @brief Initialize physics runtime for the provided scene.
         * @param pScene Scene to begin physics for.
         */
        void Begin(Scene* pScene);

        /**
         * @brief Tear down physics runtime for the provided scene.
         * @param pScene Scene to end physics for.
         */
        void End(Scene* pScene);

        /**
         * @brief Perform physics stepping for the scene (may advance internal world).
         * @param pScene Scene to step physics for.
         */
        void Step(Scene* pScene);

        /**
         * @brief Update physics-related runtime state for the scene.
         * @param pScene Scene to update.
         */
        void Update(Scene* pScene);

        /**
         * @brief Cast a ray into the physics world and return the first hit.
         * @param ray Ray to cast in world space.
         * @param result Output hit result populated on success.
         * @return True if a hit was found, false otherwise.
         */
        bool Raycast(const Ray2D& ray, HitResult2D& result) const;

	private:
		void SpawnRigidbody(class GameObject* obj);
		void HandleEvents(Scene* pScene);

		b2WorldId m_PhysicsWorldId = b2_nullWorldId;

		struct RigidbodyRuntime
		{
			b2BodyId Body = b2_nullBodyId;
			bool Valid = false;
		};

		RigidbodyRuntime* GetRuntime(GameObjectID id);

		std::unordered_map<GameObjectID, RigidbodyRuntime> m_RigidbodyRuntime;
	};
}