#include "Asset/AssetLibrary.h"

namespace Boon
{
    bool AssetLibrary::LoadPack(const std::filesystem::path& packFile)
    {
        const std::filesystem::path resolvedPath = ResolveAgainstRoots(packFile);
        if (resolvedPath.empty())
            return false;

        m_Reader = std::make_unique<AssetPackReader>();
        if (!m_Reader->Open(resolvedPath))
        {
            m_Reader.reset();
            m_Loader.reset();
            return false;
        }

        m_Loader = std::make_unique<AssetLoader>(m_Reader.get());
        return true;
    }
}