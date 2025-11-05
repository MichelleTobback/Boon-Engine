#include "Asset/AssetLibrary.h"
#include "Asset/AssetRegistry.h"

Boon::AssetLibrary::AssetLibrary(const std::string& assetDirectory)
	: m_Dirs{assetDirectory}
{
    Assets::RegisterEngineAssets(*this);
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

bool Boon::AssetLibrary::IsValidAsset(AssetHandle handle) const
{
    return m_Assets.find(handle) != m_Assets.end();
}
