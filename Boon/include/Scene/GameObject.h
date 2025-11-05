#pragma once
#include "Scene.h"
#include "GameObjectID.h"
#include "Core/Assert.h"
#include "Component/UUIDComponent.h"
#include "Component/TransformComponent.h"

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
		GameObject(GameObjectID handle, Scene* pScene);
		~GameObject();

		GameObject(const GameObject& other) = default;
		GameObject(GameObject&& other) = default;
		GameObject& operator=(const GameObject& other) = default;
		GameObject& operator=(GameObject&& other) = default;

		void SetActive(bool active);
		bool IsActive() const;

		void AttachToGameObject(GameObject parent, bool keepWorld = false);
		void DetachGameObject(GameObject child);
		GameObject GetRoot();
		GameObject GetRoot() const;

		Scene* GetScene() { return m_pScene; }

		bool IsValid() const;
		bool IsRoot() const;

		void* AddComponent(BClass* pClass);

		template <typename T, typename ... TArgs>
		T& AddComponent(TArgs&& ... args)
		{
			BN_ASSERT(!HasComponent<T>(), "GameObject already has component!");
			T& component = m_pScene->m_Registry.emplace<T>(m_Handle, std::forward<TArgs>(args)...);
			return component;
		}

		template<typename T>
		void RemoveComponent()
		{
			BN_ASSERT(HasComponent<T>(), "GameObject does not have component!");
			m_pScene->m_Registry.remove<T>(m_Handle);
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

		inline GameObject GetParent() const;
		inline const std::vector<GameObject> GetChildren() const;

		void AttachTo(GameObject parent, bool keepWorld = false);
		void Detach(GameObject child);
		void DetachFromParent();

		UUID GetUUID() const;
		TransformComponent& GetTransform();
		const TransformComponent& GetTransform() const;

	private:
		GameObjectID m_Handle{ NullGameObject };
		Scene* m_pScene{};
	};


}