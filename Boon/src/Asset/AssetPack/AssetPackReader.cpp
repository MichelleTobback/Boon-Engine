#include "Asset/AssetPack/AssetPackReader.h"
#include <cstdio>
#include <cstring>

namespace Boon
{
    AssetPackReader::~AssetPackReader()
    {
        if (m_File)
            fclose(m_File);
    }

    bool AssetPackReader::Open(const std::string& path)
    {
        m_File = fopen(path.c_str(), "rb");
        if (!m_File)
            return false;

        // Read header
        if (!ReadBytes(0, sizeof(m_AssetPack.m_Header), &m_AssetPack.m_Header))
            return false;

        if (m_AssetPack.m_Header.magic != 0x424F4F4E)
            return false;

        // Read registry
        std::vector<uint8_t> registryData(m_AssetPack.m_Header.registrySize);
        if (!ReadBytes(m_AssetPack.m_Header.registryOffset, m_AssetPack.m_Header.registrySize, registryData.data()))
            return false;

        size_t cursor = 0;
        m_AssetPack.m_Entries.clear();

        for (uint16_t i = 0; i < m_AssetPack.m_Header.assetCount; i++)
        {
            UUID id;
            memcpy(&id, &registryData[cursor], sizeof(UUID));
            cursor += sizeof(UUID);

            AssetType type;
            memcpy(&type, &registryData[cursor], sizeof(AssetType));
            cursor += sizeof(AssetType);

            uint64_t offset, size;
            memcpy(&offset, &registryData[cursor], sizeof(uint64_t));
            cursor += sizeof(uint64_t);

            memcpy(&size, &registryData[cursor], sizeof(uint64_t));
            cursor += sizeof(uint64_t);

            PackedAssetEntry entry{};
            entry.id = id;
            entry.type = type;
            entry.dataOffset = offset;
            entry.dataSize = size;

            m_AssetPack.m_Entries[id] = entry;
        }

        return true;
    }

    bool AssetPackReader::ReadAsset(AssetHandle handle, Buffer& outBuffer, AssetMeta& outMeta)
    {
        auto it = m_AssetPack.m_Entries.find(handle);
        if (it == m_AssetPack.m_Entries.end())
            return false;

        PackedAssetEntry entry = it->second;
        outMeta.uuid = handle;
        outMeta.type = entry.type;
        outBuffer.Reserve(entry.dataSize);
        ReadBytes(entry.dataOffset, entry.dataSize, outBuffer.Data());
        return true;
    }

    bool AssetPackReader::ReadBytes(uint64_t offset, uint64_t size, void* outBuffer) const
    {
        if (!m_File)
            return false;

        if (fseek(m_File, (long)offset, SEEK_SET) != 0)
            return false;

        size_t read = fread(outBuffer, 1, (size_t)size, m_File);
        return read == size;
    }
}
