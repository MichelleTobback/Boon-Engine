#include "Asset/Loader/AssetLoader.h"
#include "Asset/AssetPack/AssetPackReader.h"
#include "Asset/AssetTraits.h"
#include "Asset/AssetRegistry.h"

#include "Asset/TextureAsset.h"
#include "Asset/SpriteAtlasAsset.h"
#include "Asset/ShaderAsset.h"
#include "Asset/AssetRef.h"

#include <cassert>

namespace Boon
{
    //-----------------------------------------
    // Load: generic asset loading by UUID
    //-----------------------------------------
    Asset* AssetLoader::LoadGeneric(AssetHandle handle)
    {
        const PackedAssetEntry* entry = m_Reader->GetEntry(handle);
        if (!entry)
            return nullptr;

        return nullptr;
    }

    //-----------------------------------------
    // Load<T>()
    //-----------------------------------------
    template<typename T>
    T* AssetLoader::Load(AssetHandle handle)
    {
        const PackedAssetEntry* entry = m_Reader->GetEntry(handle);
        if (!entry)
            return nullptr;

        if (entry->type != AssetTraits<T>::Type)
            return nullptr;

        Buffer buffer{};
        AssetMeta meta{};
        if (!m_Reader->ReadAsset(handle, buffer, meta))
            return false;

        return AssetTraits<T>::Load(buffer, meta);
    }

    //-----------------------------------------
    // Explicit template instantiation
    //-----------------------------------------
    template Texture2DAsset* AssetLoader::Load<Texture2DAsset>(UUID id);
    template ShaderAsset* AssetLoader::Load<ShaderAsset>(UUID id);
    template SpriteAtlasAsset* AssetLoader::Load<SpriteAtlasAsset>(UUID id);
}
