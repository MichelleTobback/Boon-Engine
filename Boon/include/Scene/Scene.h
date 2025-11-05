#pragma once
#include "Core/UUID.h"
#include "GameObjectID.h"

#include <queue>
#include <glm/glm.hpp>
#include <unordered_map>
#include <functional>
#include <memory>

namespace Boon
{
	using SceneRegistry = entt::registry;

	struct ECSLifecycleSystem;
	class GameObject;
	class Scene final
	{
	public:
		explicit Scene();
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
		void EndUpdate();

		GameObject GetGameObject(UUID uuid);
		void ForeachGameObject(const std::function<void(GameObject)>& fn);

		void DestroyGameObject(GameObject object);

		inline SceneRegistry& GetRegistry() { return m_Registry; }

	private:
		friend class GameObject;
		//friend Scene& SceneManager::CreateScene(bool openOnLoad);
		//explicit Scene(){}

		std::unordered_map<UUID, GameObjectID> m_EntityMap;
		std::queue<UUID> m_ObjectsPendingDestroy{};
		SceneRegistry m_Registry;
		std::unique_ptr<ECSLifecycleSystem> m_pECSlifecycle;

		bool m_Running{ false };
	};
}
