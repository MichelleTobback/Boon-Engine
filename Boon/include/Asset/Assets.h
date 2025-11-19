#pragma once
#include "AssetLibrary.h"
#include "TextureAsset.h"
#include "ShaderAsset.h"
#include "SpriteAtlasAsset.h"

#include "Core/ServiceLocator.h"

namespace Boon
{
	namespace Assets
	{
		static AssetLibrary& Get()
		{
			return ServiceLocator::Get<AssetLibrary>();
		}

		static AssetRef<Texture2DAsset> GetTexture2D(AssetHandle handle)
		{
			return ServiceLocator::Get<AssetLibrary>().Load<Texture2DAsset>(handle);
		}

		static AssetRef<ShaderAsset> GetShader(AssetHandle handle)
		{
			return ServiceLocator::Get<AssetLibrary>().Load<ShaderAsset>(handle);
		}

		static AssetRef<SpriteAtlasAsset> GetSpriteAtlas(AssetHandle handle)
		{
			return ServiceLocator::Get<AssetLibrary>().Load<SpriteAtlasAsset>(handle);
		}
	}
}