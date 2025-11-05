#pragma once
#include "Assets.h"

namespace Boon
{
	namespace Assets
	{
		static void RegisterEngineAssets(AssetLibrary & lib)
		{
			lib.RegisterLoader<Texture2DAssetLoader>();
			lib.RegisterLoader<ShaderAssetLoader>();
			lib.RegisterLoader<SpriteAtlasAssetLoader>();
		}
	}
}