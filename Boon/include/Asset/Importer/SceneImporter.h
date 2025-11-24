#pragma once
#include "AssetImporter.h"
#include "Asset/SceneAsset.h"

namespace Boon
{
    class SceneImporter : public AssetImporter
    {
    public:
        using AssetType = SceneAsset;

        Asset* ImportFromFile(const std::string& filePath, const AssetMeta& meta) override
        {
            SceneAsset* pResult = new SceneAsset(meta.uuid);
            return pResult;
        }

        virtual std::vector<std::string> GetExtensions() const override
        {
            return { ".scene" };
        }
    };
}
