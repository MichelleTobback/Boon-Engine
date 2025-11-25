#pragma once
#include "Core/Boon.h"
#include "Asset/Asset.h"
#include "Asset/TextureAsset.h"

#include <glm/glm.hpp>

namespace Boon
{
	BCLASS(Name = "Texture renderer")
		struct TextureRendererComponent final
	{
		TextureRendererComponent() = default;

		BPROPERTY(ColorPicker)
		glm::vec4 Color{ 1.f };

		BPROPERTY()
		float Tiling{ 1.f };

		BPROPERTY()
		AssetRef<Texture2DAsset> Texture;
	};
}