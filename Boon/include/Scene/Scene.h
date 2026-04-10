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

	struct BClass;
	struct ECSLifecycleSystem;
	class GameObject;
	class Scene final
	{
	public:
		/**
		 * @brief Destroy the Scene.
		 *
		 * Public destructor for the scene object. See implementation for cleanup details.
		 */
		~Scene();
		Scene(const Scene& other) = delete;
		Scene(Scene&& other) = delete;
		Scene& operator=(const Scene& other) = delete;
		Scene& operator=(Scene&& other) = delete;

		/**
		 * @brief Instantiate a new game object in the scene.
		 *
		 * @param pos Initial position for the instantiated object.
		 * @return A handle to the created GameObject.
		 */
		GameObject Instantiate(const glm::vec3& pos = {});

		/**
		 * @brief Instantiate a new game object in the scene with a specific UUID.
		 *
		 * @param uuid UUID to assign to the new object.
		 * @param pos Initial position for the instantiated object.
		 * @return A handle to the created GameObject.
		 */
		GameObject Instantiate(UUID uuid, const glm::vec3& pos = {});

		/**
		 * @brief Transition the scene into the "awake" state.
		 *
		 * See implementation for the exact semantics performed during Awake.
		 */
		void Awake();

		/**
		 * @brief Transition the scene into the "sleep" state.
		 *
		 * See implementation for the exact semantics performed during Sleep.
		 */
		void Sleep();

		/**
		 * @brief Run per-frame update for the scene.
		 *
		 * See implementation for which subsystems are updated.
		 */
		void Update();

		/**
		 * @brief Run fixed-timestep updates for the scene (e.g. physics).
		 *
		 * See implementation for timestep and systems affected.
		 */
		void FixedUpdate();

		/**
		 * @brief Perform a 2D raycast against the scene's physics world.
		 *
		 * @param ray The ray to cast.
		 * @param result Output parameter populated with hit information if a hit occurs.
		 * @return true if the ray hit something and `result` was populated, false otherwise.
		 */
		bool Raycast2D(const Ray2D& ray, HitResult2D& result) const;

		/**
		 * @brief Retrieve a game object by UUID.
		 *
		 * @param uuid UUID of the requested game object.
		 * @return Handle to the corresponding GameObject. See implementation for behavior when not found.
		 */
		GameObject GetGameObject(UUID uuid);

		/**
		 * @brief Invoke a function for each game object in the scene.
		 *
		 * @param fn Function to call for each GameObject.
		 */
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

		/**
		 * @brief Destroy the given game object.
		 *
		 * See implementation for exact destruction timing and side-effects.
		 *
		 * @param object The GameObject to destroy.
		 */
		void DestroyGameObject(GameObject object);

		/**
		 * @brief Access the underlying entt registry for this scene.
		 *
		 * @return Reference to the scene's registry.
		 */
		inline SceneRegistry& GetRegistry() { return m_Registry; }

		/**
		 * @brief Get the unique id of this scene.
		 *
		 * @return SceneID assigned to this scene.
		 */
		inline SceneID GetID() const { return m_ID; }

		/**
		 * @brief Get delegate invoked when a GameObject is spawned.
		 *
		 * @return Reference to the spawn delegate.
		 */
		inline Delegate<void(GameObject)>& GetOnGameObjectSpawned() { return m_OnGameObjectSpawned; }

		/**
		 * @brief Get delegate invoked when a GameObject is destroyed.
		 *
		 * @return Reference to the destroy delegate.
		 */
		inline Delegate<void(GameObject)>& GetOnGameObjectDestroyed() { return m_OnGameObjectDestroyed; }

		/**
		 * @brief Get delegate invoked when a component is added to a GameObject.
		 *
		 * @return Reference to the component-added delegate.
		 */
		inline Delegate<void(GameObject, const BClass*)>& GetOnComponentAdded() { return m_OnComponentAdded; }

		/**
		 * @brief Get delegate invoked when a component is removed from a GameObject.
		 *
		 * @return Reference to the component-removed delegate.
		 */
		inline Delegate<void(GameObject, const BClass*)>& GetOnComponentRemoved() { return m_OnComponentRemoved; }

		/**
		 * @brief Access the ECS lifecycle system for this scene.
		 *
		 * @return Reference to the ECSLifecycleSystem instance used by the scene.
		 */
		inline ECSLifecycleSystem& GetECSLifecycleSystem() { return *m_pECSlifecycle; }

		/**
		 * @brief Query whether the scene's runtime is active.
		 *
		 * @return true when the scene is marked as running, false otherwise.
		 */
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
