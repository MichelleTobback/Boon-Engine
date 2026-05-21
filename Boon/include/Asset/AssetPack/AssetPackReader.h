#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <filesystem>

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

        bool Open(const std::filesystem::path& path);

        const PackedAssetEntry* GetEntry(UUID id) const
        {
            auto it = m_AssetPack.GetEntries().find(id);
            return (it != m_AssetPack.GetEntries().end()) ? &it->second : nullptr;
        }

        const std::unordered_map<UUID, PackedAssetEntry>& GetEntries() const
        {
            return m_AssetPack.GetEntries();
        }

        bool ReadAsset(AssetHandle handle, Buffer& outBuffer, AssetMeta& outMeta);
        bool ReadAsset(const std::filesystem::path& runtimePath, Buffer& outBuffer, AssetMeta& outMeta);

        bool ReadBytes(uint64_t offset, uint64_t size, void* outBuffer) const;

        template<typename T>
        bool ReadPOD(uint64_t offset, T& outValue) const
        {
            return ReadBytes(offset, sizeof(T), &outValue);
        }

    private:
        static std::string NormalizePathKey(const std::filesystem::path& path);

    private:
        FILE* m_File = nullptr;
        AssetPack m_AssetPack;
        std::unordered_map<std::string, AssetHandle> m_RuntimePathToHandle;
    };
}
