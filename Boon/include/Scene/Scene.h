#pragma once
#include <entt/entt.hpp>
#include "Core/UUID.h"
#include "GameObjectID.h"

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
		void LateUpdate();

		GameObject GetGameObject(UUID uuid);
		void ForeachGameObject(const std::function<void(GameObject)>& fn);

		void DestroyGameObject(GameObject object);

		inline SceneRegistry& GetRegistry() { return m_Registry; }
		inline SceneID GetID() const { return m_ID; }

	private:
		friend class GameObject;
		friend class SceneManager;
		explicit Scene(const std::string& name);

		void EndUpdate();

		std::unordered_map<UUID, GameObjectID> m_EntityMap;
		std::queue<UUID> m_ObjectsPendingDestroy{};
		SceneRegistry m_Registry;
		std::unique_ptr<ECSLifecycleSystem> m_pECSlifecycle;

		SceneID m_ID;
		std::string m_Name;
		bool m_Running{ false };
	};
}
