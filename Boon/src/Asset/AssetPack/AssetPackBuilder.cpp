#include "Asset/AssetPack/AssetPackBuilder.h"
#include <cstdio>

namespace Boon
{
    AssetPackBuilder::AssetPackBuilder(const std::string& outputPath)
        : m_OutputPath(outputPath)
    {
    }

    void AssetPackBuilder::AddAsset(UUID id, AssetType type, const Buffer& data)
    {
        m_Entries.push_back({ id, type, data });
    }

    bool AssetPackBuilder::Build()
    {
        FILE* f = fopen(m_OutputPath.c_str(), "wb");
        if (!f) return false;

        AssetPackHeader header{};
        header.magic = 0x424F4F4E;
        header.version = 1;
        header.assetCount = (uint16_t)m_Entries.size();
        header.registryOffset = sizeof(AssetPackHeader);

        // Reserve header space
        fwrite(&header, sizeof(header), 1, f);

        // Write registry placeholder
        size_t registryStart = ftell(f);

        for (const auto& e : m_Entries)
        {
            fwrite(&e.id, sizeof(UUID), 1, f);
            fwrite(&e.type, sizeof(AssetType), 1, f);

            uint64_t offset = 0, size = e.data.Size();
            fwrite(&offset, sizeof(uint64_t), 1, f);
            fwrite(&size, sizeof(uint64_t), 1, f);
        }

        size_t registryEnd = ftell(f);
        header.registrySize = registryEnd - registryStart;

        // Write actual binary data and track offsets
        std::vector<uint64_t> offsets;
        offsets.resize(m_Entries.size());

        for (size_t i = 0; i < m_Entries.size(); ++i)
        {
            offsets[i] = ftell(f);
            fwrite(m_Entries[i].data.Data(), 1, m_Entries[i].data.Size(), f);
        }

        // Patch registry with correct offsets
        fseek(f, (long)registryStart, SEEK_SET);

        for (size_t i = 0; i < m_Entries.size(); ++i)
        {
            const auto& e = m_Entries[i];

            fwrite(&e.id, sizeof(UUID), 1, f);
            fwrite(&e.type, sizeof(AssetType), 1, f);

            uint64_t offset = offsets[i];
            uint64_t size = e.data.Size();
            fwrite(&offset, sizeof(uint64_t), 1, f);
            fwrite(&size, sizeof(uint64_t), 1, f);
        }

        // Rewrite header
        fseek(f, 0, SEEK_SET);
        fwrite(&header, sizeof(header), 1, f);

        fclose(f);
        return true;
    }
}
