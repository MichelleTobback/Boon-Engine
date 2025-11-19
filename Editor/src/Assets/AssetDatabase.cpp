#include "Assets/AssetDatabase.h"

namespace BoonEditor
{
    AssetDatabase& AssetDatabase::Get()
    {
        static AssetDatabase instance;
        return instance;
    }

    void AssetDatabase::RegisterAsset(const std::string& path, AssetHandle handle)
    {
        m_HandleToPath[handle] = path;
        m_PathToHandle[path] = handle;
    }

    AssetHandle AssetDatabase::GetHandle(const std::string& path) const
    {
        if (auto it = m_PathToHandle.find(path); it != m_PathToHandle.end())
            return it->second;
        return {};
    }

    const std::string& AssetDatabase::GetPath(AssetHandle handle) const
    {
        static std::string empty;
        if (auto it = m_HandleToPath.find(handle); it != m_HandleToPath.end())
            return it->second;
        return empty;
    }

    bool AssetDatabase::Exists(AssetHandle handle) const
    {
        return m_HandleToPath.find(handle) != m_HandleToPath.end();
    }

    bool AssetDatabase::Exists(const std::string& path) const
    {
        return m_PathToHandle.find(path) != m_PathToHandle.end();
    }

    void AssetDatabase::Clear()
    {
        m_HandleToPath.clear();
        m_PathToHandle.clear();
    }
}
