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

        /**
         * @brief Import an asset from a source file.
         *
         * @param filePath Path to the source file.
         * @param meta Metadata describing the target asset.
         * @return Pointer to the created Asset on success, or nullptr on failure.
         */
        virtual Asset* ImportFromFile(const std::filesystem::path& filePath, const AssetMeta& meta) = 0;

        /**
         * @brief Export an asset to a file. Default implementation returns true.
         */
        virtual bool ExportToFile(const std::filesystem::path& filePath, Asset* asset) { return true; }

        /**
         * @brief Get supported file extensions for this importer.
         *
         * @return Vector of extension strings (without leading dot).
         */
        virtual std::vector<std::string> GetExtensions() const = 0;
    };
}
