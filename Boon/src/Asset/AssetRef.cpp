#include "Asset/AssetRef.h"

// AssetRef.cpp
#include "Asset/AssetRef.h"
#include "Asset/Assets.h"

#include "Asset/TextureAsset.h"
#include "Asset/ShaderAsset.h"
#include "Asset/SpriteAtlasAsset.h"

namespace Boon 
{
    template<typename T>
    T* AssetRef<T>::Get() const
    {
        if (!m_Handle.IsValid())
            return nullptr;
        return Assets::Get().Resolve<T>(m_Handle);
    }

    template class AssetRef<Texture2DAsset>;
    template class AssetRef<ShaderAsset>;
    template class AssetRef<SpriteAtlasAsset>;

}