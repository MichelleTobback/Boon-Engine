#pragma once
#include "AssetLibrary.h"
#include "TextureAsset.h"
#include "ShaderAsset.h"
#include "SpriteAtlasAsset.h"
#include "TilemapAsset.h"
#include "SceneAsset.h"

namespace Boon
{
	namespace Assets
	{
		static void RegisterLoaders(AssetLibrary& assetLib)
		{
			assetLib.RegisterAssetType<Texture2DAsset>();
			assetLib.RegisterAssetType<ShaderAsset>();
			assetLib.RegisterAssetType<SpriteAtlasAsset>();
			assetLib.RegisterAssetType<TilemapAsset>();
		}
	}
}