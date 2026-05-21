#include "Asset/AssetPack/AssetPackReader.h"
#include "Asset/Runtime/BAssetFile.h"

#include <cstdio>
#include <cstring>

namespace Boon
{
    AssetPackReader::~AssetPackReader()
    {
        if (m_File)
            fclose(m_File);
    }

    std::string AssetPackReader::NormalizePathKey(const std::filesystem::path& path)
    {
        std::string key = path.lexically_normal().generic_string();

        while (!key.empty() && key.front() == '/')
            key.erase(key.begin());

        return key;
    }

    bool AssetPackReader::Open(const std::filesystem::path& path)
    {
        if (m_File)
        {
            fclose(m_File);
            m_File = nullptr;
        }

        fopen_s(&m_File, path.string().c_str(), "rb");
        if (!m_File)
            return false;

        if (!ReadBytes(0, sizeof(m_AssetPack.m_Header), &m_AssetPack.m_Header))
            return false;

        if (m_AssetPack.m_Header.magic != 0x424F4F4E)
            return false;

        std::vector<uint8_t> registryData(m_AssetPack.m_Header.registrySize);
        if (!ReadBytes(m_AssetPack.m_Header.registryOffset, m_AssetPack.m_Header.registrySize, registryData.data()))
            return false;

        size_t cursor = 0;
        m_AssetPack.m_Entries.clear();
        m_RuntimePathToHandle.clear();

        for (uint16_t i = 0; i < m_AssetPack.m_Header.assetCount; ++i)
        {
            UUID id;
            std::memcpy(&id, registryData.data() + cursor, sizeof(UUID));
            cursor += sizeof(UUID);

            AssetType type;
            std::memcpy(&type, registryData.data() + cursor, sizeof(AssetType));
            cursor += sizeof(AssetType);

            uint64_t offset = 0;
            uint64_t size = 0;

            std::memcpy(&offset, registryData.data() + cursor, sizeof(uint64_t));
            cursor += sizeof(uint64_t);

            std::memcpy(&size, registryData.data() + cursor, sizeof(uint64_t));
            cursor += sizeof(uint64_t);

            std::string runtimePath{};

            if (m_AssetPack.m_Header.version >= 2)
            {
                uint32_t pathLength = 0;
                std::memcpy(&pathLength, registryData.data() + cursor, sizeof(uint32_t));
                cursor += sizeof(uint32_t);

                if (pathLength > 0)
                {
                    runtimePath.assign(
                        reinterpret_cast<const char*>(registryData.data() + cursor),
                        pathLength);
                    cursor += pathLength;
                }
            }

            PackedAssetEntry entry{};
            entry.id = id;
            entry.type = type;
            entry.dataOffset = offset;
            entry.dataSize = size;
            entry.runtimePath = runtimePath;

            m_AssetPack.m_Entries[id] = entry;

            if (!runtimePath.empty())
                m_RuntimePathToHandle[NormalizePathKey(runtimePath)] = id;
        }

        return true;
    }

    bool AssetPackReader::ReadAsset(const std::filesystem::path& runtimePath, Buffer& outBuffer, AssetMeta& outMeta)
    {
        auto it = m_RuntimePathToHandle.find(NormalizePathKey(runtimePath));
        if (it == m_RuntimePathToHandle.end())
            return false;

        return ReadAsset(it->second, outBuffer, outMeta);
    }

    bool AssetPackReader::ReadAsset(AssetHandle handle, Buffer& outBuffer, AssetMeta& outMeta)
    {
        auto it = m_AssetPack.m_Entries.find(handle);
        if (it == m_AssetPack.m_Entries.end())
            return false;

        const PackedAssetEntry& entry = it->second;

        outMeta = {};
        outMeta.uuid = handle;
        outMeta.type = entry.type;
        outMeta.runtimePath = entry.runtimePath;

        outBuffer.Clear();

        if (entry.dataSize == 0)
            return true;

        Buffer packedData{};
        packedData.Resize(static_cast<size_t>(entry.dataSize));

        if (!ReadBytes(entry.dataOffset, entry.dataSize, packedData.Data()))
            return false;

        // New packs should store the raw serialized asset payload. Older packs stored the
        // entire .basset file. Accept both so existing packs do not instantly break.
        if (packedData.Size() >= sizeof(BAssetFile::Header))
        {
            BAssetFile::Header header{};
            std::memcpy(&header, packedData.Data(), sizeof(BAssetFile::Header));

            if (header.magic == BAssetFile::Magic && header.version == BAssetFile::Version)
            {
                outMeta.uuid = AssetHandle(header.uuid);
                outMeta.type = static_cast<AssetType>(header.type);

                const size_t payloadSize = static_cast<size_t>(header.payloadSize);
                if (sizeof(BAssetFile::Header) + payloadSize > packedData.Size())
                    return false;

                outBuffer.Resize(payloadSize);
                if (payloadSize > 0)
                    std::memcpy(outBuffer.Data(), packedData.Data() + sizeof(BAssetFile::Header), payloadSize);

                return true;
            }
        }

        outBuffer = std::move(packedData);
        return true;
    }

    bool AssetPackReader::ReadBytes(uint64_t offset, uint64_t size, void* outBuffer) const
    {
        if (!m_File || !outBuffer)
            return false;

        if (fseek(m_File, static_cast<long>(offset), SEEK_SET) != 0)
            return false;

        const size_t read = fread(outBuffer, 1, static_cast<size_t>(size), m_File);
        return read == size;
    }
}
