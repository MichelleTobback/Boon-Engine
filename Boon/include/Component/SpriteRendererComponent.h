#pragma once
#include "Core/Boon.h"
#include "Asset/Asset.h"
#include "Asset/SpriteAtlasAsset.h"
#include "Asset/MaterialAsset.h"

#include <glm/glm.hpp>

#include <memory>

namespace Boon
{
	class Material;

	BCLASS(Name="Sprite renderer")
	struct SpriteRendererComponent final
	{
		SpriteRendererComponent() = default;

		BPROPERTY(ColorPicker)
		glm::vec4 Color{1.f};

		BPROPERTY()
		float Tiling{ 1.f };

		BPROPERTY()
		AssetRef<SpriteAtlasAsset> SpriteAtlasHandle;

		BPROPERTY()
		AssetRef<MaterialAsset> MaterialOverride;

		BPROPERTY(Slider, RangeMin = 0, RangeMax = 10)
		int Sprite;

		std::shared_ptr<Material> MaterialInstance = nullptr;
		AssetHandle MaterialInstanceSource = 0;
		uint32_t MaterialInstanceVersion = 0;
	};
}