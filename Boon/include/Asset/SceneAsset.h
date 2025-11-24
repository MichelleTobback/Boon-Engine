#pragma once
#include "Asset/Asset.h"
#include "Asset/AssetTraits.h"
#include "Asset/AssetMeta.h"
#include "Core/Memory/Buffer.h"
#include "Scene/Scene.h"

#include <memory>

namespace Boon
{
    class SceneAsset : public Asset
    {
    public:
        using Type = Scene;
        SceneAsset(AssetHandle handle)
            : Asset(handle) { }

        std::shared_ptr<Scene> GetInstance()
        {
            return nullptr;
        }

    private:
    };

    template<>
    struct AssetTraits<SceneAsset>
    {
        static constexpr AssetType Type = AssetType::Scene;
        static constexpr bool HasMeta = false;

        static SceneAsset* Load(Buffer& buffer, const AssetMeta& meta)
        {
            SceneAsset* asset = new SceneAsset(meta.uuid);

            return asset;
        }

        static Buffer Serialize(SceneAsset* asset)
        {
            Buffer out;

            return out;
        }
    };

}
