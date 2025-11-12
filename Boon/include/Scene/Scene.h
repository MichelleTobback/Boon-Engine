#pragma once
#include <entt/entt.hpp>
#include "Core/UUID.h"
#include "GameObjectID.h"
#include "Physics/PhysicsWorld2D.h"

#include <queue>
#include <glm/glm.hpp>
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>

namespace Boon
{
	using SceneRegistry = entt::registry;
	using SceneID = UUID;

	struct ECSLifecycleSystem;
	class GameObject;
	class Scene final
	{
	public:
		~Scene();
		Scene(const Scene& other) = delete;
		Scene(Scene&& other) = delete;
		Scene& operator=(const Scene& other) = delete;
		Scene& operator=(Scene&& other) = delete;

		GameObject Instantiate(const glm::vec3& pos = {});
		GameObject Instantiate(UUID uuid, const glm::vec3& pos = {});

		void Awake();
		void Sleep();
		void Update();
		void FixedUpdate();

		bool Raycast2D(const Ray2D& ray, HitResult2D& result) const;

		GameObject GetGameObject(UUID uuid);
		void ForeachGameObject(const std::function<void(GameObject)>& fn);
		void ForeachGameObject(const std::function<void(GameObject)>& fn) const;

		template<typename... Components>
		auto GetAllGameObjectsWith()
		{
			return m_Registry.view<Components...>();
		}

		void DestroyGameObject(GameObject object);

		inline SceneRegistry& GetRegistry() { return m_Registry; }
		inline SceneID GetID() const { return m_ID; }

	private:
		friend class PhysicsWorld2D;
		friend class GameObject;
		friend class SceneManager;
		friend class SceneSerializer;
		explicit Scene(const std::string& name);

		GameObject Instantiate(UUID uuid, GameObjectID id);

		void OnUpdate();
		void LateUpdate();

		void OnBeginOverlap(GameObject overlapped, GameObject other);
		void OnEndOverlap(GameObject overlapped, GameObject other);

		std::unordered_map<UUID, GameObjectID> m_EntityMap;
		std::queue<UUID> m_ObjectsPendingDestroy{};
		SceneRegistry m_Registry;
		std::unique_ptr<ECSLifecycleSystem> m_pECSlifecycle;
		PhysicsWorld2D m_Physics2D;

		SceneID m_ID;
		std::string m_Name;
		bool m_Running{ false };
	};
}
