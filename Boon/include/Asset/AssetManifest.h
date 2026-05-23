#pragma once

#include "Asset/AssetMeta.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace Boon
{
    struct AssetManifestEntry
    {
        AssetHandle uuid = UUID::Null;
        AssetType type = AssetType::None;

        // Path used by game/editor code, for example "textures/player.png".
        std::filesystem::path logicalPath{};

        // Path inside the runtime asset root or pack, for example "textures/player.basset".
        std::filesystem::path runtimePath{};

        bool IsValid() const
        {
            return uuid.IsValid() && type != AssetType::None && !runtimePath.empty();
        }
    };

    class AssetManifest final
    {
    public:
        void Add(const AssetManifestEntry& entry);
        void Add(const AssetMeta& meta);

        bool Remove(AssetHandle handle);

        const AssetManifestEntry* Get(AssetHandle handle) const;
        const AssetManifestEntry* GetByLogicalPath(const std::filesystem::path& path) const;
        const AssetManifestEntry* GetByRuntimePath(const std::filesystem::path& path) const;

        bool Contains(AssetHandle handle) const;
        bool Empty() const { return m_ByHandle.empty(); }

        void Clear();

        const std::unordered_map<AssetHandle, AssetManifestEntry>& GetAll() const
        {
            return m_ByHandle;
        }

        bool LoadFromFile(const std::filesystem::path& path);
        bool SaveToFile(const std::filesystem::path& path) const;

        static std::string NormalizePathKey(const std::filesystem::path& path);

    private:
        std::unordered_map<AssetHandle, AssetManifestEntry> m_ByHandle;
        std::unordered_map<std::string, AssetHandle> m_LogicalPathToHandle;
        std::unordered_map<std::string, AssetHandle> m_RuntimePathToHandle;
    };
}
