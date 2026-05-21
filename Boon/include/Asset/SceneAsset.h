#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetMeta.h"
#include "Asset/AssetSerializer.h"
#include "Asset/AssetTraits.h"
#include "Core/Memory/Buffer.h"
#include "Scene/Scene.h"

namespace Boon
{
    class SceneAsset : public Asset
    {
    public:
        using Type = Scene;

        explicit SceneAsset(AssetHandle handle)
            : Asset(handle)
        {
        }

        Scene* GetInstance()
        {
            return nullptr;
        }
    };

    template<>
    struct AssetTraits<SceneAsset>
    {
        static constexpr AssetType Type = AssetType::Scene;
        static constexpr const char* Name = "Scene";
    };

    template<>
    struct AssetSerializer<SceneAsset>
    {
        static SceneAsset* Load(Buffer&, const AssetMeta& meta)
        {
            return new SceneAsset(meta.uuid);
        }

        static Buffer Serialize(SceneAsset*)
        {
            return {};
        }
    };
}
