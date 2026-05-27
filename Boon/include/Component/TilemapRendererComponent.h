#include "Reflection/BClassBase.h"
#include "Asset/TilemapAsset.h"
#include "Asset/MaterialAsset.h"
#include "Asset/AssetRef.h"

#include <memory>

namespace Boon
{
	BCLASS(Name="Tilemap renderer")
	struct TilemapRendererComponent
	{
		BPROPERTY()
		AssetRef<TilemapAsset> tilemap;

		BPROPERTY()
		AssetRef<MaterialAsset> MaterialOverride;

		std::shared_ptr<Material> MaterialInstance = nullptr;
		AssetHandle MaterialInstanceSource = 0;
		uint32_t MaterialInstanceVersion = 0;
	};
}