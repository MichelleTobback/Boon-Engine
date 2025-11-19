#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include "Asset/Asset.h"
#include "Core/UUID.h"
#include "Asset/AssetRegistry.h"
#include "Asset/AssetPack/AssetPack.h"

namespace Boon
{
    class AssetPackReader
    {
    public:
        AssetPackReader() = default;
        ~AssetPackReader();

        bool Open(const std::string& path);

        const PackedAssetEntry* GetEntry(UUID id) const
        {
            auto it = m_AssetPack.GetEntries().find(id);
            return (it != m_AssetPack.GetEntries().end()) ? &it->second : nullptr;
        }

        bool ReadAsset(AssetHandle handle, Buffer& outBuffer, AssetMeta& outMeta);

        bool ReadBytes(uint64_t offset, uint64_t size, void* outBuffer) const;

        template<typename T>
        bool ReadPOD(uint64_t offset, T& outValue) const
        {
            return ReadBytes(offset, sizeof(T), &outValue);
        }

    private:
        FILE* m_File = nullptr;
        AssetPack m_AssetPack;
    };
}
