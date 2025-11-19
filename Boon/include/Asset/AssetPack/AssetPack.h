#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>

#include "Core/UUID.h"
#include "Core/Memory/Buffer.h"
#include "Asset/Asset.h"

namespace Boon
{
    // Binary pack header written once at the front of the file.
    struct AssetPackHeader
    {
        uint32_t magic = 0x424F4F4E; // "BOON"
        uint16_t version = 1;
        uint16_t assetCount = 0;
        uint64_t registryOffset = 0;
        uint64_t registrySize = 0;
    };

    // Binary metadata for one asset in the pack.
    struct PackedAssetEntry
    {
        UUID id;
        AssetType type;
        uint64_t dataOffset;
        uint64_t dataSize;
    };

    class AssetPack final
    {
    public:
        inline const AssetPackHeader& GetHeader() const { return m_Header; }
        inline const std::unordered_map<UUID, PackedAssetEntry>& GetEntries() const { return m_Entries; }
        inline Buffer& GetBuffer() { return m_Data; }

    private:
        friend class AssetPackBuilder;
        friend class AssetPackReader;

        AssetPackHeader m_Header;
        std::unordered_map<UUID, PackedAssetEntry> m_Entries;
        Buffer m_Data;
    };
}
