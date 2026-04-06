#pragma once
#include <entt/entt.hpp>
#include "Core/UUID.h"
#include "Core/Delegate.h"
#include "GameObjectID.h"
#include "Physics/PhysicsWorld2D.h"
#include "Reflection/BClass.h"

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

	class BClass;
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

		template<typename... Components>
		void ForeachGameObjectWith(const std::function<void(GameObject)>& fn)
		{
			auto view = m_Registry.view<Components...>();
			for (auto ent : view)
			{
				GameObject obj{ ent, this };
				fn(obj);
			}
		}

		template<typename... Components>
		auto GetAllGameObjectsWith()
		{
			return m_Registry.view<Components...>();
		}

		void DestroyGameObject(GameObject object);

		inline SceneRegistry& GetRegistry() { return m_Registry; }
		inline SceneID GetID() const { return m_ID; }

		inline Delegate<void(GameObject)>& GetOnGameObjectSpawned() { return m_OnGameObjectSpawned; }
		inline Delegate<void(GameObject)>& GetOnGameObjectDestroyed() { return m_OnGameObjectDestroyed; }
		inline Delegate<void(GameObject, const BClass*)>& GetOnComponentAdded() { return m_OnComponentAdded; }
		inline Delegate<void(GameObject, const BClass*)>& GetOnComponentRemoved() { return m_OnComponentRemoved; }

		inline ECSLifecycleSystem& GetECSLifecycleSystem() { return *m_pECSlifecycle; }

		inline bool IsRunning() const { return m_Running; }

	private:
		friend class PhysicsWorld2D;
		friend class GameObject;
		friend class SceneManager;
		friend class SceneSerializer;
		explicit Scene(const std::string& name);

		GameObject Instantiate(UUID uuid, GameObjectID id);

		void OnUpdate();
		void LateUpdate();
		void EndUpdate();

		void OnBeginOverlap(GameObject overlapped, GameObject other);
		void OnEndOverlap(GameObject overlapped, GameObject other);

		template <typename T, typename ... TArgs>
		T& AddComponent(GameObjectID handle, TArgs&& ... args);
		void AwakeComponent(GameObjectID handle, const BClass* pClass);

		std::unique_ptr<ECSLifecycleSystem> m_pECSlifecycle;
		PhysicsWorld2D m_Physics2D;

		SceneRegistry m_Registry;

		std::unordered_map<UUID, GameObjectID> m_EntityMap;
		std::queue<UUID> m_ObjectsPendingDestroy{};

		Delegate<void(GameObject)> m_OnGameObjectSpawned;
		Delegate<void(GameObject)> m_OnGameObjectDestroyed;
		Delegate<void(GameObject, const BClass*)> m_OnComponentAdded;
		Delegate<void(GameObject, const BClass*)> m_OnComponentRemoved;

		struct SceneCommand
		{
			enum class Type
			{
				AwakeComponent,
				RemoveComponent
			} Type;

			GameObjectID GameObjectId;
			const BClass* pClass;
		};

		std::queue<SceneCommand> m_Commands;

		void PushCommand(const SceneCommand& cmd);
		void ProcessCommands();

		SceneID m_ID;
		std::string m_Name;
		bool m_Running{ false };
	};

	template <typename T, typename ... TArgs>
	T& Scene::AddComponent(GameObjectID handle, TArgs&& ... args)
	{
		T& comp = m_Registry.emplace<T>(handle, std::forward<TArgs>(args)...);

		const BClass* cls = BClassRegistry::Get().Find<T>();
		//PushCommand({ SceneCommand::Type::AwakeComponent, handle, cls });

		if (cls)
		{
			GameObject obj{ handle, this };
			if (m_Running && cls->awake)
				cls->awake(obj);

			m_OnComponentAdded.Invoke(obj, cls);
		}

		return comp;
	}
}
