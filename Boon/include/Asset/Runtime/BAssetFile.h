#pragma once

#include <cstdint>
#include <filesystem>

#include "Asset/AssetMeta.h"
#include "Core/Memory/Buffer.h"

namespace Boon
{
    class BAssetFile final
    {
    public:
        static constexpr uint32_t Magic = 0x42534154; // 'BSAT'
        static constexpr uint32_t Version = 1;

        struct Header
        {
            uint32_t magic = Magic;
            uint32_t version = Version;
            uint32_t type = 0;
            uint32_t reserved = 0;
            uint64_t uuid = 0;
            uint64_t payloadSize = 0;
        };

        static bool Write(const std::filesystem::path& path, const AssetMeta& meta, const Buffer& payload);
        static bool Read(const std::filesystem::path& path, AssetMeta& outMeta, Buffer& outPayload);
    };
}
