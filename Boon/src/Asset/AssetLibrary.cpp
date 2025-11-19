#include "Asset/AssetLibrary.h"

namespace Boon
{
    bool AssetLibrary::LoadPack(const std::string& packFile)
    {
        m_Reader = std::make_unique<AssetPackReader>();
        if (!m_Reader->Open(packFile))
            return false;

        m_Loader = std::make_unique<AssetLoader>(m_Reader.get());
        return true;
    }
}
