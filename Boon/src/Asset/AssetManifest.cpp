#include "Asset/AssetManifest.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace Boon
{
    std::string AssetManifest::NormalizePathKey(const std::filesystem::path& path)
    {
        std::filesystem::path normalized = path.lexically_normal();

        std::string key = normalized.generic_string();

        while (!key.empty() && key.front() == '/')
            key.erase(key.begin());

        return key;
    }

    void AssetManifest::Add(const AssetManifestEntry& entry)
    {
        if (!entry.IsValid())
            return;

        AssetManifestEntry stored = entry;
        stored.logicalPath = stored.logicalPath.lexically_normal();
        stored.runtimePath = stored.runtimePath.lexically_normal();

        m_ByHandle[stored.uuid] = stored;

        if (!stored.logicalPath.empty())
            m_LogicalPathToHandle[NormalizePathKey(stored.logicalPath)] = stored.uuid;

        if (!stored.runtimePath.empty())
            m_RuntimePathToHandle[NormalizePathKey(stored.runtimePath)] = stored.uuid;
    }

    void AssetManifest::Add(const AssetMeta& meta)
    {
        AssetManifestEntry entry{};
        entry.uuid = meta.uuid;
        entry.type = meta.type;
        entry.logicalPath = meta.sourcePath;
        entry.runtimePath = meta.runtimePath;

        Add(entry);
    }

    const AssetManifestEntry* AssetManifest::Get(AssetHandle handle) const
    {
        auto it = m_ByHandle.find(handle);
        return it == m_ByHandle.end() ? nullptr : &it->second;
    }

    const AssetManifestEntry* AssetManifest::GetByLogicalPath(const std::filesystem::path& path) const
    {
        const auto key = NormalizePathKey(path);

        auto it = m_LogicalPathToHandle.find(key);
        if (it == m_LogicalPathToHandle.end())
            return nullptr;

        return Get(it->second);
    }

    const AssetManifestEntry* AssetManifest::GetByRuntimePath(const std::filesystem::path& path) const
    {
        const auto key = NormalizePathKey(path);

        auto it = m_RuntimePathToHandle.find(key);
        if (it == m_RuntimePathToHandle.end())
            return nullptr;

        return Get(it->second);
    }

    bool AssetManifest::Contains(AssetHandle handle) const
    {
        return m_ByHandle.find(handle) != m_ByHandle.end();
    }

    void AssetManifest::Clear()
    {
        m_ByHandle.clear();
        m_LogicalPathToHandle.clear();
        m_RuntimePathToHandle.clear();
    }

    bool AssetManifest::LoadFromFile(const std::filesystem::path& path)
    {
        std::ifstream file(path);
        if (!file)
            return false;

        nlohmann::json json{};
        file >> json;

        if (!json.contains("assets") || !json["assets"].is_array())
            return false;

        for (const auto& item : json["assets"])
        {
            AssetManifestEntry entry{};
            entry.uuid = AssetHandle(item.value("uuid", uint64_t{ 0 }));
            entry.type = static_cast<AssetType>(item.value("type", static_cast<uint32_t>(AssetType::None)));
            entry.logicalPath = item.value("logicalPath", std::string{});
            entry.runtimePath = item.value("runtimePath", std::string{});

            Add(entry);
        }

        return true;
    }

    bool AssetManifest::SaveToFile(const std::filesystem::path& path) const
    {
        std::filesystem::create_directories(path.parent_path());

        nlohmann::json json{};
        json["version"] = 1;
        json["assets"] = nlohmann::json::array();

        for (const auto& [handle, entry] : m_ByHandle)
        {
            nlohmann::json item{};
            item["uuid"] = static_cast<uint64_t>(entry.uuid);
            item["type"] = static_cast<uint32_t>(entry.type);
            item["logicalPath"] = entry.logicalPath.generic_string();
            item["runtimePath"] = entry.runtimePath.generic_string();

            json["assets"].push_back(item);
        }

        std::ofstream file(path, std::ios::trunc);
        if (!file)
            return false;

        file << json.dump(4);
        return static_cast<bool>(file);
    }
}
