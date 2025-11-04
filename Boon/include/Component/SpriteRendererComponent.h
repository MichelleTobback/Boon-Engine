#pragma once
#include "Asset/Asset.h"
#include <glm/glm.hpp>

namespace Boon
{
	struct SpriteRendererComponent final
	{
		SpriteRendererComponent() = default;

		glm::vec4 Color{1.f};
		float Tiling{ 1.f };
		AssetHandle SpriteAtlasHandle;
		int Sprite;
	};
}