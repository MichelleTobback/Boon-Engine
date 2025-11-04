#include "Asset/AssetLibrary.h"
#include "Asset/ShaderAsset.h"
#include "Asset/TextureAsset.h"

Boon::AssetLibrary::AssetLibrary(const std::string& assetDirectory)
	: m_Dirs{assetDirectory}
{
    RegisterLoader<ShaderAssetLoader>();
    RegisterLoader<Texture2DAssetLoader>();
}

Boon::AssetLibrary::~AssetLibrary()
{
    m_ExtensionsToLoaders.clear();
    m_Paths.clear();
    m_TypesToHandles.clear();
    m_Assets.clear();
    m_AssetLoaders.clear();
}

void Boon::AssetLibrary::AddDirectory(const std::string& directory)
{
    m_Dirs.push_back(directory);
}
