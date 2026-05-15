#pragma once
#include "Asset/Asset.h"
#include "Asset/AssetTraits.h"
#include "Asset/AssetMeta.h"
#include "Core/Memory/Buffer.h"

#include <memory>

namespace Boon
{
    class PrefabAsset : public Asset
    {
    public:
        using Type = Scene;
        PrefabAsset(AssetHandle handle)
            : Asset(handle) {
        }

        std::shared_ptr<Prefab> GetInstance()
        {
            return nullptr;
        }

    private:
    };

    template<>
    struct AssetTraits<PrefabAsset>
    {
        static constexpr AssetType Type = AssetType::Scene;
        static constexpr bool HasMeta = false;

        static PrefabAsset* Load(Buffer&, const AssetMeta& meta)
        {
            PrefabAsset* asset = new PrefabAsset(meta.uuid);

            return asset;
        }

        static Buffer Serialize(PrefabAsset*)
        {
            Buffer out;

            return out;
        }
    };

}
