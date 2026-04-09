#pragma once

#include <memory>
#include <vector>
#include <filesystem>
#include <initializer_list>

#include "AssetPack/AssetCache.h"
#include "AssetPack/AssetPackReader.h"
#include "Asset/Loader/AssetLoader.h"
#include "Asset/AssetRef.h"

#include "Core/ServiceLocator.h"
#include "Asset/Importer/AssetImporterRegistry.h"

namespace Boon
{
    class AssetLibrary
    {
    public:
        explicit AssetLibrary(const std::filesystem::path& root)
        {
            InitializeImporterRegistryBindings();
            AddRoot(root);
        }

        explicit AssetLibrary(const std::vector<std::filesystem::path>& roots)
        {
            InitializeImporterRegistryBindings();
            for (const auto& root : roots)
            {
                AddRoot(root);
            }
        }

        AssetLibrary(std::initializer_list<std::filesystem::path> roots)
        {
            InitializeImporterRegistryBindings();
            for (const auto& root : roots)
            {
                AddRoot(root);
            }
        }

        /**
         * @brief Add an asset root directory.
         *
         * Roots are searched in insertion order.
         * The first root is also the default import destination.
         */
        void AddRoot(const std::filesystem::path& root)
        {
            if (root.empty())
                return;

            m_Roots.push_back(root.lexically_normal());
        }

        /**
         * @brief Replace all roots with a single root.
         */
        void SetRoot(const std::filesystem::path& root)
        {
            m_Roots.clear();
            AddRoot(root);
        }

        /**
         * @brief Replace all roots.
         */
        void SetRoots(const std::vector<std::filesystem::path>& roots)
        {
            m_Roots.clear();
            for (const auto& root : roots)
            {
                AddRoot(root);
            }
        }

        /**
         * @brief Remove all configured roots.
         */
        void ClearRoots()
        {
            m_Roots.clear();
        }

        /**
         * @brief Get the primary/default root.
         *
         * Returns an empty path if no roots are configured.
         */
        const std::filesystem::path& GetRoot() const
        {
            static const std::filesystem::path s_EmptyPath{};
            return m_Roots.empty() ? s_EmptyPath : m_Roots.front();
        }

        /**
         * @brief Get all configured roots.
         */
        const std::vector<std::filesystem::path>& GetRoots() const
        {
            return m_Roots;
        }

        /**
         * @brief Load an asset pack from disk.
         *
         * If packFile is relative, all roots are searched in order.
         * If packFile is absolute, it is opened directly.
         *
         * @param packFile Path to the asset pack file.
         * @return true if the pack was loaded, false otherwise.
         */
        bool LoadPack(const std::filesystem::path& packFile);

        template<typename T>
        T* Resolve(AssetHandle handle)
        {
            if (auto cached = m_Cache.Find<T>(handle))
                return cached;

            if (!m_Loader)
                return nullptr;

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

            if (!m_Loader)
                return AssetRef<T>();

            T* result = m_Loader->Load<T>(handle);
            if (!result)
                return AssetRef<T>();

            m_Cache.Store(result);
            return AssetRef<T>(result->GetHandle());
        }

        /**
         * @brief Clear the internal asset cache.
         */
        void ClearCache() { m_Cache.Clear(); }

        /**
         * @brief Check whether an asset handle corresponds to a registered asset.
         *
         * @param handle Asset handle to query.
         * @return true if the registry contains metadata for the handle, false otherwise.
         */
        bool IsValidAsset(AssetHandle handle) const
        {
            return m_Registry.Get(handle) != nullptr;
        }

        /**
         * @brief Import an asset into the default root (first configured root).
         *
         */
        template<typename T>
        AssetRef<T> Import(const std::filesystem::path& filepath)
        {
            AssetImporterRegistry::Imported<T> result =
                ServiceLocator::Get<AssetImporterRegistry>().ImportAndLoad<T>(ResolveAgainstRoots(filepath));

            if (!result.asset)
                return AssetRef<T>();

            m_Cache.Store(result.asset);
            m_Registry.Add(result.meta);
            return AssetRef<T>(result.meta.uuid);
        }

        /**
         * @brief Import an asset into a specific root index.
         */
        template<typename T>
        AssetRef<T> ImportToRoot(const std::filesystem::path& filepath, size_t rootIndex)
        {
            if (rootIndex >= m_Roots.size())
                return AssetRef<T>();

            AssetImporterRegistry::Imported<T> result =
                ServiceLocator::Get<AssetImporterRegistry>().ImportAndLoad<T>(m_Roots[rootIndex] / filepath);

            if (!result.asset)
                return AssetRef<T>();

            m_Cache.Store(result.asset);
            m_Registry.Add(result.meta);
            return AssetRef<T>(result.meta.uuid);
        }

        const AssetMeta* GetMeta(AssetHandle handle) const
        {
            return m_Registry.Get(handle);
        }

    private:
        void InitializeImporterRegistryBindings()
        {
            auto& reg = ServiceLocator::Get<AssetImporterRegistry>();
            reg.m_pCache = &m_Cache;
            reg.m_pRegistry = &m_Registry;
        }

        const std::filesystem::path* GetPrimaryRootInternal() const
        {
            return m_Roots.empty() ? nullptr : &m_Roots.front();
        }

        std::filesystem::path ResolveAgainstRoots(const std::filesystem::path& path) const
        {
            if (path.empty())
                return {};

            if (path.is_absolute())
                return path.lexically_normal();

            for (const auto& root : m_Roots)
            {
                const std::filesystem::path candidate = (root / path).lexically_normal();
                if (std::filesystem::exists(candidate))
                    return candidate;
            }

            return {};
        }

    private:
        AssetCache m_Cache;
        AssetRegistry m_Registry;
        std::unique_ptr<AssetPackReader> m_Reader;
        std::unique_ptr<AssetLoader> m_Loader;
        std::vector<std::filesystem::path> m_Roots;
    };
}