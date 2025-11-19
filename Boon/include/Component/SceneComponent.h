#pragma once
#include "Core/Boon.h"
#include "Scene/GameObject.h"

namespace Boon
{
	class SceneComponent final
	{
	public:
		class Iterator
		{
		public:
			using iterator_category = std::forward_iterator_tag;
			using value_type = GameObject;
			using difference_type = std::ptrdiff_t;
			using pointer = GameObject*;
			using reference = GameObject&;

			Iterator(std::vector<GameObjectID>::iterator it, Scene* scene)
				: m_Iter(it), m_pScene(scene) {}

			GameObject operator*() const
			{
				return GameObject(*m_Iter, m_pScene);
			}

			Iterator& operator++()
			{
				++m_Iter;
				return *this;
			}

			bool operator!=(const Iterator& other) const
			{
				return m_Iter != other.m_Iter;
			}

		private:
			std::vector<GameObjectID>::iterator m_Iter;
			Scene* m_pScene{};
		};

		Iterator begin() { return Iterator(m_Children.begin(), m_pScene); }
		Iterator end() { return Iterator(m_Children.end(), m_pScene); }

		SceneComponent() = default;
		SceneComponent(GameObject owner, GameObject parent = GameObject());

		SceneComponent& operator=(const SceneComponent& other);

		inline GameObject GetOwner() const { return GameObject(m_Owner, m_pScene); }
		inline GameObject GetParent() const { return GameObject(m_Parent, m_pScene); }
		inline const std::vector<GameObjectID> GetChildren() const { return m_Children; }

		SceneComponent GetRoot();
		SceneComponent GetRoot() const;

		GameObject GetRootObject();
		GameObject GetRootObject() const;

		bool IsRoot() const;
		bool HasChildren() const;

		void AttachTo(SceneComponent& parent, bool keepWorld = false);
		void Detach(SceneComponent& child);

	private:
		void AddChild(GameObject child);
		void RemoveChild(GameObject child);
		void SetScene(Scene* pScene);

		friend class TransformComponent;
		friend class SceneSerializer;

		GameObjectID m_Owner{NullGameObject};
		GameObjectID m_Parent{ NullGameObject };
		std::vector<GameObjectID> m_Children{};
		Scene* m_pScene{};
	};
}