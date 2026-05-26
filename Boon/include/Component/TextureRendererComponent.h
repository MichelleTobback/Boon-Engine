#pragma once
#include "Core/Boon.h"
#include "Asset/Asset.h"
#include "Asset/TextureAsset.h"

#include <glm/glm.hpp>
#include <memory>

namespace Boon
{
	class Material;

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

		std::shared_ptr<Material> MaterialOverride = nullptr;
	};
}