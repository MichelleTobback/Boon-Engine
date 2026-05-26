#pragma once

#include "Assets/Importer/AssetImporter.h"
#include "Asset/Runtime/BAssetFile.h"
#include "Asset/SceneAsset.h"

namespace Boon
{
    class SceneImporter : public AssetImporter
    {
    public:
        virtual ~SceneImporter() = default;

        using AssetType = SceneAsset;

        bool ImportToBAsset(
            AssetLibrary& assetLib,
            const std::filesystem::path& sourcePath,
            const std::filesystem::path& exportPath,
            const AssetMeta& meta) override
        {
            SceneAsset asset(meta.uuid);
            Buffer payload = AssetSerializer<SceneAsset>::Serialize(&asset);
            return BAssetFile::Write(exportPath, meta, payload);
        }

        std::vector<std::string> GetExtensions() const override
        {
            return { ".scene" };
        }
    };
}
