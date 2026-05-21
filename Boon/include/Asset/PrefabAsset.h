#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetMeta.h"
#include "Asset/AssetSerializer.h"
#include "Asset/AssetTraits.h"
#include "Core/Memory/Buffer.h"
#include "Scene/Prefab.h"

#include <memory>

namespace Boon
{
    class PrefabAsset : public Asset
    {
    public:
        using Type = Prefab;

        explicit PrefabAsset(AssetHandle handle)
            : Asset(handle)
        {
        }

        std::shared_ptr<Prefab> GetInstance()
        {
            return nullptr;
        }
    };

    template<>
    struct AssetTraits<PrefabAsset>
    {
        static constexpr AssetType Type = AssetType::Prefab;
        static constexpr const char* Name = "Prefab";
    };

    template<>
    struct AssetSerializer<PrefabAsset>
    {
        static PrefabAsset* Load(Buffer&, const AssetMeta& meta)
        {
            return new PrefabAsset(meta.uuid);
        }

        static Buffer Serialize(PrefabAsset*)
        {
            return {};
        }
    };
}
