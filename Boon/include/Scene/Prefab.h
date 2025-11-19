#include <Core/Memory/Buffer.h>
#include "Reflection/BClass.h"

#include <unordered_map>

namespace Boon
{
	class Prefab final
	{
	public:
		template<typename T>
		T* GetComponent()
		{
			BClass* cls = BClassRegistry::Get().Find<T>();
			if (!cls)
				return nullptr;

			auto it = m_Components.find(cls->hash);
			if (it == m_Components.end())
				return nullptr;

			T* comp = it->second.Read<T>();
		}

	private:
		std::unordered_map<uint32_t, Buffer> m_Components;
	};
}