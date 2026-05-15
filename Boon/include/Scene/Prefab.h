#include <Core/Memory/Buffer.h>
#include "Reflection/BClass.h"
#include "Scene/GameObject.h"

#include <unordered_map>

namespace Boon
{
	class Prefab final
	{
	public:
		void ApplyTo(GameObject obj);

	private:
		struct PrefabNode
		{
			std::unordered_map<BClass*, Buffer> Components;
			std::vector<PrefabNode> Children;
			GameObjectID Id{};
		};

		void ResolveNode(PrefabNode& node);

		PrefabNode m_Root{};
	};
}