#pragma once
#include "AssetLibrary.h"
#include "TextureAsset.h"
#include "ShaderAsset.h"
#include "SpriteAtlasAsset.h"
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
	}
}