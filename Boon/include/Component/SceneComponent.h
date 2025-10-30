#pragma once

#include "Scene/GameObject.h"

namespace Boon
{
	class SceneComponent final
	{
	public:
		SceneComponent(GameObject owner, GameObject parent = GameObject());

		inline GameObject GetOwner() const { return m_Owner; }
		inline GameObject GetParent() const { return m_Parent; }
		inline const std::vector<GameObject> GetChildren() const { return m_Children; }

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

		friend class TransformComponent;

		GameObject m_Owner{};
		GameObject m_Parent{};
		std::vector<GameObject> m_Children{};
	};
}