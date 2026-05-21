#pragma once

#include <filesystem>

#include "Asset/AssetMeta.h"
#include "Core/Memory/Buffer.h"

namespace Boon
{
    class IAssetSource
    {
    public:
        virtual ~IAssetSource() = default;

        virtual bool Read(const std::filesystem::path& runtimePath, Buffer& outPayload, AssetMeta& outMeta) = 0;
        virtual bool Read(AssetHandle handle, Buffer& outPayload, AssetMeta& outMeta) = 0;
    };
}
