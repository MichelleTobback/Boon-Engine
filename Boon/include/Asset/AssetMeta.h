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

        /**
         * @brief Check whether this metadata entry describes a valid asset.
         *
         * @return true if uuid is valid and type is not None.
         */
        bool IsValid() const { return uuid.IsValid() && type != AssetType::None; }
    };
}
