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

		static std::shared_ptr<Texture2D> GetTexture2D(AssetHandle handle)
		{
			return ServiceLocator::Get<AssetLibrary>().GetAsset<Texture2DAsset>(handle);
		}

		static std::shared_ptr<Shader> GetShader(AssetHandle handle)
		{
			return ServiceLocator::Get<AssetLibrary>().GetAsset<ShaderAsset>(handle);
		}

		static std::shared_ptr<SpriteAtlas> GetSpriteAtlas(AssetHandle handle)
		{
			return ServiceLocator::Get<AssetLibrary>().GetAsset<SpriteAtlasAsset>(handle);
		}
	}
}