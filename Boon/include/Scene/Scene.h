#pragma once
#include "Core/UUID.h"
#include "GameObjectID.h"

#include <queue>
#include <glm/glm.hpp>
#include <unordered_map>

namespace Boon
{
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
		void EndUpdate();

		GameObject GetGameObject(UUID uuid);

		void DestroyGameObject(GameObject object);

	private:
		friend class GameObject;
		//friend Scene& SceneManager::CreateScene(bool openOnLoad);
		friend class Application;
		explicit Scene(){}

		std::unordered_map<UUID, GameObjectID> m_EntityMap;
		std::queue<UUID> m_ObjectsPendingDestroy{};
		entt::registry m_Registry;

		bool m_Running{ false };
	};
}
