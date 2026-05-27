#pragma once
#include "Core/Boon.h"
#include "Asset/Asset.h"
#include "Asset/TextureAsset.h"
#include "Asset/MaterialAsset.h"

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

		BPROPERTY()
		AssetRef<MaterialAsset> MaterialOverride;

		std::shared_ptr<Material> MaterialInstance = nullptr;
		AssetHandle MaterialInstanceSource = 0;
		uint32_t MaterialInstanceVersion = 0;
	};
}