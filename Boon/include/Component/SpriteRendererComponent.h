#pragma once
#include "Core/Boon.h"
#include "Asset/Asset.h"
#include "Asset/SpriteAtlasAsset.h"

#include <glm/glm.hpp>

namespace Boon
{
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

		BPROPERTY(Slider, RangeMin=0, RangeMax=10)
		int Sprite;
	};
}