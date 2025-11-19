#pragma once
#include "Asset/AssetType.h"

namespace Boon
{
    template<typename T>
    struct AssetTraits
    {
        static constexpr AssetType Type = AssetType::None;
        static constexpr const char* Name = "Unknown";
    };
}
