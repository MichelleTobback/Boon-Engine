#pragma once
#include "Asset/Asset.h"
#include "Asset/AssetMeta.h"
#include <string>
#include <memory>
#include <vector>

namespace Boon
{
    class AssetImporter
    {
    public:
        virtual ~AssetImporter() = default;

        virtual Asset* ImportFromFile(const std::string& filePath, const AssetMeta& meta) = 0;
        virtual std::vector<std::string> GetExtensions() const = 0;
    };
}
