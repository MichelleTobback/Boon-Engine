#include "Reflection/BClassBase.h"
#include "Asset/TilemapAsset.h"
#include "Asset/AssetRef.h"

#include <memory>

namespace Boon
{
	class Material;

	BCLASS(Name="Tilemap renderer")
	struct TilemapRendererComponent
	{
		BPROPERTY()
		AssetRef<TilemapAsset> tilemap;

		std::shared_ptr<Material> MaterialOverride = nullptr;

		std::shared_ptr<Material> MaterialInstance = nullptr;
	};
}