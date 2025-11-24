#include "Reflection/BClassBase.h"
#include "Asset/TilemapAsset.h"
#include "Asset/AssetRef.h"

namespace Boon
{
	BCLASS(Name="Tilemap renderer")
	struct TilemapRendererComponent
	{
		BPROPERTY()
		AssetRef<TilemapAsset> tilemap;
	};
}