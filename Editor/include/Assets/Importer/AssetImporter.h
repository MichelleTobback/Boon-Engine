#pragma once
#include "Asset/Asset.h"
#include "Asset/AssetMeta.h"
#include <string>
#include <memory>
#include <vector>
#include <filesystem>

namespace Boon
{
    class AssetImporter
    {
    public:
        virtual ~AssetImporter() = default;

        virtual bool ImportToBAsset(
            AssetLibrary& assetLib,
            const std::filesystem::path& sourcePath,
            const std::filesystem::path& exportPath,
            const AssetMeta& meta) = 0;

        virtual bool ExportToFile(const std::filesystem::path&, Asset*)
        {
            return true;
        }

        virtual std::vector<std::string> GetExtensions() const = 0;
    };
}
