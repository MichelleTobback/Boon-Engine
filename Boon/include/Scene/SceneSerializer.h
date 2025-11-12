#pragma once
#include "Scene.h"

namespace Boon
{
	class SceneSerializer final
	{
	public:
		SceneSerializer(Scene& scene);

		void Serialize(const std::string& dst);
		void Deserialize(const std::string& src);

		void Clear();
		void Copy(Scene& from);

	private:
		Scene& m_Context;
	};
}