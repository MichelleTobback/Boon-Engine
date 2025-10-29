#include "Asset/AssetLibrary.h"
#include "Asset/ShaderAsset.h"

Boon::AssetLibrary::AssetLibrary(const std::string& assetDirectory)
	: m_Root{assetDirectory}
{
    RegisterLoader<ShaderAssetLoader>();
}

Boon::AssetLibrary::~AssetLibrary()
{
    m_ExtensionsToLoaders.clear();
    m_Paths.clear();
    m_TypesToHandles.clear();
    m_Assets.clear();
    m_AssetLoaders.clear();
}
