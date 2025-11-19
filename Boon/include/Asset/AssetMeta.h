#pragma once
#include "Core/UUID.h"
#include "AssetType.h"
#include <string>
#include <unordered_map>

namespace Boon
{
    struct AssetMeta
    {
        AssetHandle uuid = UUID::Null;
        AssetType type = AssetType::None;

        std::unordered_map<std::string, std::string> settings;
        std::vector<AssetHandle> dependencies;

        bool IsValid() const { return uuid.IsValid() && type != AssetType::None; }
    };
}
