#pragma once
#include "Asset/Asset.h"
#include <glm/glm.hpp>

namespace Boon
{
	struct SpriteRendererComponent final
	{
		SpriteRendererComponent() = default;

		glm::vec4 Color{1.f};
		glm::vec4 TexRect{0.f, 0.f, 1.f, 1.f};
		float Tiling{ 1.f };
		AssetHandle TextureHandle;
	};
}