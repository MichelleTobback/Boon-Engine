#pragma once

#include <cstdint>

namespace Boon
{
    enum class AssetType : uint32_t
    {
        None = 0,
        Texture,
        Shader,
        SpriteAtlas,
        Scene,
        Prefab,
        Audio,

        COUNT
    };
}
