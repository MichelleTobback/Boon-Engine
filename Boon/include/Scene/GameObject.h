#pragma once
#include "Scene.h"
#include "GameObjectID.h"
#include "Core/Assert.h"
#include "Component/UUIDComponent.h"
#include "Component/TransformComponent.h"
#include "Reflection/BClass.h"

#include <memory>

namespace Boon
{
	enum class GameObjectFlag
	{
		None = 0,
		Active = 1,
		Valid = 2
	};

	struct GameObjectDataComponent
	{
		GameObjectFlag Flags;
	};

	struct BClass;
	class TransformComponent;
	class GameObject final
	{
	public:
		GameObject() = default;

		/**
		 * @brief Construct a GameObject wrapper for an entity handle in a scene.
		 *
		 * @param handle The entity handle identifying the object in the scene registry.
		 * @param pScene Pointer to the Scene that owns the entity.
		 */
		GameObject(GameObjectID handle, Scene* pScene);

		/**
		 * @brief Destroy the GameObject wrapper.
		 *
		 * This is the public destructor for the handle wrapper. See implementation for cleanup.
		 */
		~GameObject();

		GameObject(const GameObject& other) = default;
		GameObject(GameObject&& other) = default;
		GameObject& operator=(const GameObject& other) = default;
		GameObject& operator=(GameObject&& other) = default;

		/**
		 * @brief Set the active flag for this game object.
		 *
		 * @param active New active state.
		 */
		void SetActive(bool active);

		/**
		 * @brief Query the active flag of this game object.
		 *
		 * @return true if the object is marked active, false otherwise.
		 */
		bool IsActive() const;

		/**
		 * @brief Attach this object to a parent game object.
		 *
		 * @param parent Parent GameObject to attach to.
		 * @param keepWorld If true, attempt to preserve world transform. See implementation for details.
		 */
		void AttachToGameObject(GameObject parent, bool keepWorld = false);

		/**
		 * @brief Detach the specified child object from this object.
		 *
		 * @param child Child GameObject to detach.
		 */
		void DetachGameObject(GameObject child);

		/**
		 * @brief Get the root ancestor of this object.
		 *
		 * @return The root GameObject in this object's hierarchy.
		 */
		GameObject GetRoot();

		/**
		 * @brief Get the root ancestor of this object (const overload).
		 *
		 * @return The root GameObject in this object's hierarchy.
		 */
		GameObject GetRoot() const;

		/**
		 * @brief Get the scene that owns this GameObject.
		 *
		 * @return Pointer to the owning Scene.
		 */
		Scene* GetScene() { return m_pScene; }

		/**
		 * @brief Check whether this GameObject references a valid entity.
		 *
		 * @return true if the wrapper refers to a valid entity handle, false otherwise.
		 */
		bool IsValid() const;

		/**
		 * @brief Check whether this GameObject is a root (has no parent).
		 *
		 * @return true if this object is a root, false otherwise.
		 */
		bool IsRoot() const;

		/**
		 * @brief Add a component to this GameObject by runtime BClass description.
		 *
		 * @param pClass Pointer to the runtime class describing the component type.
		 * @return Pointer to the created component storage.
		 */
		void* AddComponentFromClass(const BClass* pClass);

		/**
		 * @brief Retrieve a component by runtime BClass description.
		 *
		 * @param pClass Pointer to the runtime class describing the component type.
		 * @return Pointer to the component if present, otherwise implementation-defined.
		 */
		void* GetComponentByClass(const BClass* pClass);

		/**
		 * @brief Get a component by class, or add it if not present.
		 *
		 * @param pClass Runtime class descriptor for the component.
		 * @return Pointer to the existing or newly added component.
		 */
		void* GetOrAddComponentByClass(const BClass* pClass);

		/**
		 * @brief Check whether this GameObject has a component matching the runtime class.
		 *
		 * @param pClass Runtime class descriptor.
		 * @return true if the component exists on the object, false otherwise.
		 */
		bool HasComponentByClass(const BClass* pClass);

		/**
		 * @brief Remove a component from this GameObject by runtime class.
		 *
		 * @param pClass Runtime class descriptor for the component to remove.
		 */
		void RemoveComponentByClass(const BClass* pClass);

		template <typename T, typename ... TArgs>
		T& AddComponent(TArgs&& ... args)
		{
			static_assert(!std::is_empty_v<T>,
				"Component type T is empty. Empty components are stored as tags by entt and "
				"cannot be added as normal components. Add at least one data member.");

			BN_ASSERT(!HasComponent<T>(), "GameObject already has component!");

			return m_pScene->AddComponent<T>(m_Handle, std::forward<TArgs>(args)...);
		}

		template <typename T, typename ... TArgs>
		T& GetOrAddComponent(TArgs&& ... args)
		{
			if (HasComponent<T>())
				return GetComponent<T>();

			return AddComponent<T>();
		}

		template<typename T>
		void RemoveComponent()
		{
			BN_ASSERT(HasComponent<T>(), "GameObject does not have component!");
			m_pScene->m_Registry.remove<T>(m_Handle);

			const BClass* cls = BClassRegistry::Get().Find<T>();
			if (cls)
				m_pScene->m_OnComponentRemoved.Invoke(*this, cls);
		}

		template <typename T>
		const T& GetComponent() const
		{
			BN_ASSERT(HasComponent<T>(), "GameObject does not have component!");
			return m_pScene->m_Registry.get<T>(m_Handle);
		}

		template <typename T>
		T& GetComponent()
		{
			BN_ASSERT(HasComponent<T>(), "GameObject does not have component!");
			return m_pScene->m_Registry.get<T>(m_Handle);
		}

		template <typename T>
		bool HasComponent() const
		{
			return m_pScene->m_Registry.any_of<T>(m_Handle);
		}

		template <typename T>
		const T* TryGetComponentInParent() const
		{
			if (HasComponent<T>())
				return &GetComponent<T>();

			GameObject parent = GetParent();

			if (!parent.IsValid())
				return nullptr;

			return parent.TryGetComponentInParent<T>();
		}

		template <typename T>
		GameObject TryGetParentWithComponent() const
		{
			if (HasComponent<T>())
				return *this;

			GameObject parent = GetParent();

			if (!parent.IsValid())
				return GameObject();

			return parent.TryGetParentWithComponent<T>();
		}

		/**
		 * @brief Mark or perform destruction of the underlying entity.
		 *
		 * See implementation for exact destruction semantics and timing.
		 */
		void Destroy();

		operator bool() const { return m_Handle != NullGameObject; }
		operator GameObjectID() const { return m_Handle; }
		operator uint32_t() const { return (uint32_t)m_Handle; }

		bool operator==(const GameObject& other) const
		{
			return m_Handle == other.m_Handle && m_pScene == other.m_pScene;
		}

		bool operator!=(const GameObject& other) const
		{
			return !(*this == other);
		}

		/**
		 * @brief Get the parent GameObject of this object.
		 *	@return Parent GameObject, or an invalid GameObject if none.
		 */
		GameObject GetParent() const;

		/**
		 * @brief Get the list of children for this GameObject.
		 *
		 * @return Vector containing child GameObjects. Ownership remains with the scene.
		 */
		const std::vector<GameObject> GetChildren() const;

		/**
		 * @brief Attach this object to a parent (alternative API to AttachToGameObject).
		 *
		 * @param parent Parent GameObject to attach to.
		 * @param keepWorld If true, attempt to preserve world transform. See implementation.
		 */
		void AttachTo(GameObject parent, bool keepWorld = false);

		/**
		 * @brief Detach the given child from this object (alternative API to DetachGameObject).
		 *
		 * @param child Child to detach.
		 */
		void Detach(GameObject child);

		/**
		 * @brief Detach this object from its parent.
		 */
		void DetachFromParent();

		/**
		 * @brief Get the UUID associated with this GameObject.
		 *
		 * @return UUID for the object as stored in its UUID component.
		 */
		UUID GetUUID() const;

		/**
		 * @brief Access the transform component for this object.
		 *
		 * @return Reference to the object's TransformComponent.
		 */
		TransformComponent& GetTransform();

		/**
		 * @brief Const access to the transform component for this object.
		 *
		 * @return Const reference to the object's TransformComponent.
		 */
		const TransformComponent& GetTransform() const;

	private:
		GameObjectID m_Handle{ NullGameObject };
		Scene* m_pScene{};
		SceneID m_SceneID{0u};
	};


}