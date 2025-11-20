#include "Assets/AssetDatabase.h"
#include "Asset/Assets.h"
#include <algorithm>

namespace BoonEditor
{
    namespace Utils
    {
        static inline std::string NormalizePath(std::string path)
        {
            // Replace backslashes with forward slashes
            std::replace(path.begin(), path.end(), '\\', '/');

            // Remove duplicate slashes ("//")
            while (path.find("//") != std::string::npos)
                path = path.replace(path.find("//"), 2, "/");

            // Remove trailing slash (optional)
            if (path.size() > 1 && path.back() == '/')
                path.pop_back();

            return path;
        }
    }

    AssetDatabase& AssetDatabase::Get()
    {
        static AssetDatabase instance;
        return instance;
    }

    AssetDatabase::AssetDatabase()
    {
        Init();
    }

    void AssetDatabase::Init()
    {
        m_DefaultTextures[AssetType::Texture] = Assets::Get().Import<Texture2DAsset>("Icons/Assets/texture_icon.png");
        m_DefaultTextures[AssetType::SpriteAtlas] = Assets::Get().Import<Texture2DAsset>("Icons/Assets/sprite_atlas_icon.png");
        m_DefaultTextures[AssetType::Shader] = Assets::Get().Import<Texture2DAsset>("Icons/Assets/shader_icon.png");
    }

    void AssetDatabase::RegisterAsset(const std::string& path, AssetHandle handle)
    {
        std::string cleanPath = Utils::NormalizePath(path);
        m_HandleToPath[handle] = cleanPath;
        m_PathToHandle[cleanPath] = handle;
        m_Dirty = true;
    }

    AssetHandle AssetDatabase::GetHandle(const std::string& path) const
    {
        if (auto it = m_PathToHandle.find(Utils::NormalizePath(path)); it != m_PathToHandle.end())
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
        return m_PathToHandle.find(Utils::NormalizePath(path)) != m_PathToHandle.end();
    }

    void AssetDatabase::Clear()
    {
        m_HandleToPath.clear();
        m_PathToHandle.clear();
    }

    void AssetDatabase::ForEachEntry(const std::function<void(AssetHandle, const std::string&)>& fn)
    {
        for (auto [handle, path] : m_HandleToPath)
        {
            fn(handle, path);
        }
    }

    AssetRef<Texture2DAsset> AssetDatabase::GetThumbnail(AssetHandle handle) const
    {
        const AssetMeta* meta = Assets::Get().GetMeta(handle);
        if (!meta)
            return AssetRef<Texture2DAsset>();

        if (meta->type == AssetType::Texture)
            return Assets::Get().Load<Texture2DAsset>(handle);

        auto it = m_DefaultTextures.find(meta->type);
        if (it == m_DefaultTextures.end())
            return AssetRef<Texture2DAsset>();

        return it->second;
    }
}
