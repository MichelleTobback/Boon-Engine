#pragma once

#include <filesystem>
#include <functional>
#include <initializer_list>
#include <memory>
#include <vector>

#include "Asset/AssetManifest.h"
#include "Asset/AssetPack/AssetCache.h"
#include "Asset/AssetPack/AssetPackReader.h"
#include "Asset/AssetRef.h"
#include "Asset/AssetRegistry.h"
#include "Asset/AssetTraits.h"
#include "Asset/Loader/AssetLoaderRegistry.h"
#include "Asset/Source/FolderAssetSource.h"
#include "Asset/Source/IAssetSource.h"
#include "Asset/Source/PackAssetSource.h"

namespace Boon
{
    class AssetLibrary
    {
    public:
        AssetLibrary();
        explicit AssetLibrary(const std::filesystem::path& runtimeAssetRoot);
        explicit AssetLibrary(const std::vector<std::filesystem::path>& runtimeAssetRoots);
        AssetLibrary(std::initializer_list<std::filesystem::path> runtimeAssetRoots);
        ~AssetLibrary();

        template<typename T>
        void RegisterAssetType()
        {
            m_Loaders.Register<T>();
        }

        void RegisterBuiltinAssetTypes();

        void SetRuntimeAssetRoot(const std::filesystem::path& root);
        void AddRuntimeAssetRoot(const std::filesystem::path& root);
        void SetRuntimeAssetRoots(const std::vector<std::filesystem::path>& roots);
        void ClearRuntimeAssetRoots();

        const std::filesystem::path& GetRuntimeAssetRoot() const;
        const std::vector<std::filesystem::path>& GetRuntimeAssetRoots() const;

        // Compatibility wrappers while old call sites are migrated.
        void SetRoot(const std::filesystem::path& root) { SetRuntimeAssetRoot(root); }
        void AddRoot(const std::filesystem::path& root) { AddRuntimeAssetRoot(root); }
        void SetRoots(const std::vector<std::filesystem::path>& roots) { SetRuntimeAssetRoots(roots); }
        void ClearRoots() { ClearRuntimeAssetRoots(); }
        const std::filesystem::path& GetRoot() const { return GetRuntimeAssetRoot(); }
        const std::vector<std::filesystem::path>& GetRoots() const { return GetRuntimeAssetRoots(); }

        bool LoadManifest(const std::filesystem::path& manifestFile);
        bool LoadPack(const std::filesystem::path& packFile);

        template<typename T>
        AssetRef<T> Load(const std::filesystem::path& sourceOrRuntimePath)
        {
            const AssetManifestEntry* entry = ResolveManifestEntry(sourceOrRuntimePath);
            if (!entry)
                return AssetRef<T>();

            if (entry->type != AssetTraits<T>::Type)
                return AssetRef<T>();

            Asset* asset = LoadFromManifestEntry(*entry, AssetTraits<T>::Type);
            if (!asset)
                return AssetRef<T>();

            return AssetRef<T>(asset->GetHandle());
        }

        template<typename T>
        AssetRef<T> Load(AssetHandle handle)
        {
            if (auto cached = m_Cache.Find<T>(handle))
                return AssetRef<T>(cached->GetHandle());

            Asset* asset = ResolveUntyped(handle, AssetTraits<T>::Type);
            if (!asset)
                return AssetRef<T>();

            return AssetRef<T>(asset->GetHandle());
        }

        template<typename T>
        T* Resolve(AssetHandle handle)
        {
            return static_cast<T*>(ResolveUntyped(handle, AssetTraits<T>::Type));
        }

        Asset* ResolveUntyped(AssetHandle handle, AssetType expectedType);

        const AssetMeta* GetMeta(AssetHandle handle) const;
        bool IsValidAsset(AssetHandle handle) const;

        void RegisterMeta(const AssetMeta& meta);
        void RegisterManifestEntry(const AssetManifestEntry& entry);

        const AssetManifest& GetManifest() const { return m_Manifest; }
        AssetManifest& GetManifest() { return m_Manifest; }

        AssetRegistry& GetRegistry() { return m_Registry; }
        const AssetRegistry& GetRegistry() const { return m_Registry; }

        AssetLoaderRegistry& GetLoaderRegistry() { return m_Loaders; }
        const AssetLoaderRegistry& GetLoaderRegistry() const { return m_Loaders; }

        void ClearCache();
        void ClearRegistry();

        using MissingAssetCallback = std::function<bool(const std::filesystem::path& logicalPath)>;

        // Editor only: lets the editor generate a missing .basset before runtime loading.
        // Runtime/shipping builds should not bind this.
        void BindMissingAssetCallback(MissingAssetCallback callback)
        {
            m_MissingAssetCallback = std::move(callback);
        }

        std::filesystem::path ResolveRuntimePathOnDisk(const std::filesystem::path& runtimePath) const;

    private:
        void BindAssetRefResolver();

        const AssetManifestEntry* ResolveManifestEntry(const std::filesystem::path& logicalOrRuntimePath);
        Asset* LoadFromManifestEntry(const AssetManifestEntry& entry, AssetType expectedType);
        bool EnsureRuntimeAssetExists(const AssetManifestEntry& entry);

    private:
        AssetCache m_Cache;
        AssetRegistry m_Registry;
        AssetManifest m_Manifest;
        AssetLoaderRegistry m_Loaders;

        std::vector<std::unique_ptr<IAssetSource>> m_Sources;
        std::vector<std::filesystem::path> m_RuntimeAssetRoots;

        MissingAssetCallback m_MissingAssetCallback;
    };
}
