#pragma once
#include "AssetLibrary.h"
#include "TextureAsset.h"
#include "ShaderAsset.h"
#include "SpriteAtlasAsset.h"
#include "TilemapAsset.h"
#include "SceneAsset.h"

#include "Core/ServiceLocator.h"

namespace Boon
{
	namespace Assets
	{
		static AssetLibrary& Get()
		{
			return ServiceLocator::Get<AssetLibrary>();
		}

		static void RegisterLoaders()
		{
			AssetLibrary& assetLib = Get();
			assetLib.RegisterAssetType<Texture2DAsset>();
			assetLib.RegisterAssetType<ShaderAsset>();
			assetLib.RegisterAssetType<SpriteAtlasAsset>();
			assetLib.RegisterAssetType<TilemapAsset>();
		}

		[[maybe_unused]]
		static AssetRef<Texture2DAsset> GetTexture2D(AssetHandle handle)
		{
			return ServiceLocator::Get<AssetLibrary>().Load<Texture2DAsset>(handle);
		}

		[[maybe_unused]]
		static AssetRef<ShaderAsset> GetShader(AssetHandle handle)
		{
			return ServiceLocator::Get<AssetLibrary>().Load<ShaderAsset>(handle);
		}

		[[maybe_unused]]
		static AssetRef<SpriteAtlasAsset> GetSpriteAtlas(AssetHandle handle)
		{
			return ServiceLocator::Get<AssetLibrary>().Load<SpriteAtlasAsset>(handle);
		}

		[[maybe_unused]]
		static AssetRef<TilemapAsset> GetTilemap(AssetHandle handle)
		{
			return ServiceLocator::Get<AssetLibrary>().Load<TilemapAsset>(handle);
		}
	}
}