#pragma once

#include "Asset/AssetType.h"
#include "Core/UUID.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Boon
{
    using AssetHandle = UUID;

    struct AssetMeta
    {
        AssetHandle uuid = UUID::Null;
        AssetType type = AssetType::None;

        std::filesystem::path sourcePath{};
        std::filesystem::path runtimePath{};
        std::unordered_map<std::string, std::string> settings{};
        std::vector<AssetHandle> dependencies{};

        bool IsValid() const
        {
            return uuid.IsValid() && type != AssetType::None;
        }
    };
}