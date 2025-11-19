#pragma once
#include <memory>

#include "AssetPack/AssetCache.h"
#include "AssetPack/AssetPackReader.h"
#include "Asset/Loader/AssetLoader.h"
#include "Asset/AssetRef.h"

#include "Asset/Importer/AssetImporterRegistry.h"

namespace Boon
{
    class AssetLibrary
    {
    public:
        AssetLibrary(const std::string& root)
            : m_Root(root) 
        {
            auto& reg = AssetImporterRegistry::Get(); 
            reg.m_pCache = &m_Cache;
            reg.m_pRegistry = &m_Registry;
        }
        bool LoadPack(const std::string& packFile);

        template<typename T>
        T* Resolve(AssetHandle handle)
        {
            if (auto cached = m_Cache.Find<T>(handle))
                return cached;

            T* loaded = m_Loader->Load<T>(handle);
            if (!loaded)
                return nullptr;

            m_Cache.Store(loaded);
            return loaded;
        }

        template<typename T>
        AssetRef<T> Load(AssetHandle handle)
        {
            if (auto cached = m_Cache.Find<T>(handle))
                return AssetRef<T>(cached->GetHandle());

            T* result = m_Loader->Load<T>(handle);
            if (!result)
                return AssetRef<T>();

            m_Cache.Store(result);
            return AssetRef<T>(result->GetHandle());
        }

        void ClearCache() { m_Cache.Clear(); }

        bool IsValidAsset(AssetHandle handle) const
        {
            return m_Registry.Get(handle);
        }

        template<typename T>
        AssetRef<T> Import(const std::string& filepath)
        {
            AssetImporterRegistry::Imported<T> result = AssetImporterRegistry::Get().ImportAndLoad<T>(m_Root + filepath);
            m_Cache.Store(result.asset);
            m_Registry.Add(result.meta);
            return AssetRef<T>(result.meta.uuid);
        }

    private:
        AssetCache m_Cache;
        AssetRegistry m_Registry;
        std::unique_ptr<AssetPackReader> m_Reader;
        std::unique_ptr<AssetLoader> m_Loader;
        std::string m_Root;
    };
}
