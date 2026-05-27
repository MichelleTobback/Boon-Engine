#include "Asset/AssetLibrary.h"

#include "Asset/ShaderAsset.h"
#include "Asset/SpriteAtlasAsset.h"
#include "Asset/TextureAsset.h"
#include "Asset/TilemapAsset.h"
#include "Asset/SceneAsset.h"
#include "Asset/PrefabAsset.h"
#include "Asset/MaterialAsset.h"

namespace Boon
{
    AssetLibrary::AssetLibrary()
    {
        BindAssetRefResolver();
        RegisterBuiltinAssetTypes();
    }

    AssetLibrary::AssetLibrary(const std::filesystem::path& runtimeAssetRoot)
        : AssetLibrary()
    {
        SetRuntimeAssetRoot(runtimeAssetRoot);
    }

    AssetLibrary::AssetLibrary(const std::vector<std::filesystem::path>& runtimeAssetRoots)
        : AssetLibrary()
    {
        SetRuntimeAssetRoots(runtimeAssetRoots);
    }

    AssetLibrary::AssetLibrary(std::initializer_list<std::filesystem::path> runtimeAssetRoots)
        : AssetLibrary()
    {
        for (const auto& root : runtimeAssetRoots)
            AddRuntimeAssetRoot(root);
    }

    AssetLibrary::~AssetLibrary()
    {
        AssetRefResolver::Unbind();
    }

    void AssetLibrary::BindAssetRefResolver()
    {
        AssetRefResolver::Bind(
            [this](AssetHandle handle, AssetType expectedType) -> Asset*
            {
                return ResolveUntyped(handle, expectedType);
            });
    }

    void AssetLibrary::RegisterBuiltinAssetTypes()
    {
        RegisterAssetType<Texture2DAsset>();
        RegisterAssetType<ShaderAsset>();
        RegisterAssetType<SpriteAtlasAsset>();
        RegisterAssetType<TilemapAsset>();
        RegisterAssetType<SceneAsset>();
        RegisterAssetType<PrefabAsset>();
        RegisterAssetType<MaterialAsset>();
    }

    void AssetLibrary::SetRuntimeAssetRoot(const std::filesystem::path& root)
    {
        ClearRuntimeAssetRoots();
        AddRuntimeAssetRoot(root);
    }

    void AssetLibrary::AddRuntimeAssetRoot(const std::filesystem::path& root)
    {
        if (root.empty())
            return;

        std::filesystem::path normalized = root.lexically_normal();

        m_RuntimeAssetRoots.push_back(normalized);
        m_Sources.push_back(std::make_unique<FolderAssetSource>(normalized));
    }

    void AssetLibrary::SetRuntimeAssetRoots(const std::vector<std::filesystem::path>& roots)
    {
        ClearRuntimeAssetRoots();

        for (const auto& root : roots)
            AddRuntimeAssetRoot(root);
    }

    void AssetLibrary::ClearRuntimeAssetRoots()
    {
        m_RuntimeAssetRoots.clear();
        m_Sources.clear();
    }

    const std::filesystem::path& AssetLibrary::GetRuntimeAssetRoot() const
    {
        static const std::filesystem::path s_EmptyPath{};
        return m_RuntimeAssetRoots.empty() ? s_EmptyPath : m_RuntimeAssetRoots.front();
    }

    const std::vector<std::filesystem::path>& AssetLibrary::GetRuntimeAssetRoots() const
    {
        return m_RuntimeAssetRoots;
    }

    bool AssetLibrary::LoadManifest(const std::filesystem::path& manifestFile)
    {
        std::filesystem::path resolvedPath = manifestFile;

        if (!resolvedPath.is_absolute())
        {
            resolvedPath.clear();

            for (const auto& root : m_RuntimeAssetRoots)
            {
                const std::filesystem::path candidate = (root / manifestFile).lexically_normal();
                if (std::filesystem::exists(candidate))
                {
                    resolvedPath = candidate;
                    break;
                }
            }
        }

        if (resolvedPath.empty() || !std::filesystem::exists(resolvedPath))
            return false;

        if (!m_Manifest.LoadFromFile(resolvedPath))
            return false;

        for (const auto& [handle, entry] : m_Manifest.GetAll())
        {
            AssetMeta meta{};
            meta.uuid = entry.uuid;
            meta.type = entry.type;
            meta.sourcePath = entry.logicalPath;
            meta.runtimePath = entry.runtimePath;
            m_Registry.Add(meta);
        }

        return true;
    }

    bool AssetLibrary::LoadPack(const std::filesystem::path& packFile)
    {
        std::filesystem::path resolvedPath = packFile;

        if (!resolvedPath.is_absolute())
        {
            resolvedPath.clear();

            for (const auto& root : m_RuntimeAssetRoots)
            {
                const std::filesystem::path candidate = (root / packFile).lexically_normal();
                if (std::filesystem::exists(candidate))
                {
                    resolvedPath = candidate;
                    break;
                }
            }
        }

        if (resolvedPath.empty() || !std::filesystem::exists(resolvedPath))
            return false;

        auto reader = std::make_unique<AssetPackReader>();
        if (!reader->Open(resolvedPath))
            return false;

        for (const auto& [handle, packedEntry] : reader->GetEntries())
        {
            if (!packedEntry.id.IsValid() || packedEntry.type == AssetType::None || packedEntry.runtimePath.empty())
                continue;

            AssetManifestEntry entry{};
            entry.uuid = packedEntry.id;
            entry.type = packedEntry.type;
            entry.logicalPath = packedEntry.runtimePath;
            entry.runtimePath = packedEntry.runtimePath;
            RegisterManifestEntry(entry);
        }

        m_Sources.push_back(std::make_unique<PackAssetSource>(std::move(reader)));
        return true;
    }

    Asset* AssetLibrary::ResolveUntyped(AssetHandle handle, AssetType expectedType)
    {
        if (!handle.IsValid() || expectedType == AssetType::None)
            return nullptr;

        const AssetManifestEntry* entry = m_Manifest.Get(handle);
        if (!entry || entry->type != expectedType)
            return nullptr;

        if (Asset* cached = m_Cache.FindUntyped(handle))
            return cached;

        return LoadFromManifestEntry(*entry, expectedType);
    }

    const AssetMeta* AssetLibrary::GetMeta(AssetHandle handle) const
    {
        return m_Registry.Get(handle);
    }

    bool AssetLibrary::IsValidAsset(AssetHandle handle) const
    {
        return m_Manifest.Contains(handle) || m_Registry.Get(handle) != nullptr;
    }

    void AssetLibrary::RegisterMeta(const AssetMeta& meta)
    {
        if (!meta.IsValid())
            return;

        m_Registry.Add(meta);
        m_Manifest.Add(meta);
    }

    void AssetLibrary::RegisterManifestEntry(const AssetManifestEntry& entry)
    {
        if (!entry.IsValid())
            return;

        m_Manifest.Add(entry);

        AssetMeta meta{};
        meta.uuid = entry.uuid;
        meta.type = entry.type;
        meta.sourcePath = entry.logicalPath;
        meta.runtimePath = entry.runtimePath;
        m_Registry.Add(meta);
    }

    void AssetLibrary::ClearCache()
    {
        m_Cache.Clear();
    }

    void AssetLibrary::ClearRegistry()
    {
        m_Registry.Clear();
        m_Manifest.Clear();
    }

    std::filesystem::path AssetLibrary::ResolveRuntimePathOnDisk(const std::filesystem::path& runtimePath) const
    {
        if (runtimePath.empty())
            return {};

        if (runtimePath.is_absolute())
            return std::filesystem::exists(runtimePath) ? runtimePath.lexically_normal() : std::filesystem::path{};

        for (const auto& root : m_RuntimeAssetRoots)
        {
            const std::filesystem::path candidate = (root / runtimePath).lexically_normal();
            if (std::filesystem::exists(candidate))
                return candidate;
        }

        return {};
    }

    const AssetManifestEntry* AssetLibrary::ResolveManifestEntry(const std::filesystem::path& logicalOrRuntimePath)
    {
        if (logicalOrRuntimePath.empty())
            return nullptr;

        if (const AssetManifestEntry* entry = m_Manifest.GetByLogicalPath(logicalOrRuntimePath))
            return entry;

        if (const AssetManifestEntry* entry = m_Manifest.GetByRuntimePath(logicalOrRuntimePath))
            return entry;

        // Editor/dev safety net. The editor callback may import the source asset and register a manifest entry.
        if (m_MissingAssetCallback && m_MissingAssetCallback(logicalOrRuntimePath))
        {
            if (const AssetManifestEntry* entry = m_Manifest.GetByLogicalPath(logicalOrRuntimePath))
                return entry;

            if (const AssetManifestEntry* entry = m_Manifest.GetByRuntimePath(logicalOrRuntimePath))
                return entry;
        }

        // Direct .basset loading fallback for loose runtime assets without a manifest.
        if (logicalOrRuntimePath.extension() == ".basset")
        {
            std::filesystem::path resolved = ResolveRuntimePathOnDisk(logicalOrRuntimePath);
            if (!resolved.empty())
            {
                AssetMeta meta{};
                Buffer payload{};
                for (auto& source : m_Sources)
                {
                    if (source->Read(logicalOrRuntimePath, payload, meta) && meta.IsValid())
                    {
                        AssetManifestEntry entry{};
                        entry.uuid = meta.uuid;
                        entry.type = meta.type;
                        entry.logicalPath = logicalOrRuntimePath;
                        entry.runtimePath = logicalOrRuntimePath;

                        RegisterManifestEntry(entry);
                        return m_Manifest.Get(entry.uuid);
                    }
                }
            }
        }

        return nullptr;
    }

    Asset* AssetLibrary::LoadFromManifestEntry(const AssetManifestEntry& entry, AssetType expectedType)
    {
        if (!entry.IsValid() || entry.type != expectedType)
            return nullptr;

        if (Asset* cached = m_Cache.FindUntyped(entry.uuid))
            return cached;

        IAssetLoader* loader = m_Loaders.Get(entry.type);
        if (!loader)
            return nullptr;

        auto loadFromSourcePayload = [&](Buffer& payload, AssetMeta& meta) -> Asset*
        {
            if (meta.type != expectedType)
                return nullptr;

            if (meta.uuid.IsValid() && meta.uuid != entry.uuid)
                return nullptr;

            meta.uuid = entry.uuid;
            meta.type = entry.type;
            meta.sourcePath = entry.logicalPath;
            meta.runtimePath = entry.runtimePath;

            std::unique_ptr<Asset> loaded = loader->Load(payload, meta);
            if (!loaded)
                return nullptr;

            Asset* raw = m_Cache.StoreUntyped(std::move(loaded));
            m_Registry.Add(meta);
            return raw;
        };

        auto tryReadFromSources = [&]() -> Asset*
        {
            Buffer payload{};
            AssetMeta meta{};

            for (auto& source : m_Sources)
            {
                payload.Clear();
                meta = {};

                if (!source->Read(entry.runtimePath, payload, meta))
                    continue;

                if (Asset* loaded = loadFromSourcePayload(payload, meta))
                    return loaded;

                return nullptr;
            }

            // Packs can also be handle-addressed. This is useful when the runtime path is
            // not present or when loading from a pack-generated manifest entry.
            for (auto& source : m_Sources)
            {
                payload.Clear();
                meta = {};

                if (!source->Read(entry.uuid, payload, meta))
                    continue;

                if (Asset* loaded = loadFromSourcePayload(payload, meta))
                    return loaded;

                return nullptr;
            }

            return nullptr;
        };

        if (Asset* loaded = tryReadFromSources())
            return loaded;

        // Editor/dev safety net: generate the missing .basset, then try sources again.
        if (m_MissingAssetCallback && !entry.logicalPath.empty() && m_MissingAssetCallback(entry.logicalPath))
            return tryReadFromSources();

        return nullptr;
    }

    bool AssetLibrary::EnsureRuntimeAssetExists(const AssetManifestEntry& entry)
    {
        if (entry.runtimePath.empty())
            return false;

        if (!ResolveRuntimePathOnDisk(entry.runtimePath).empty())
            return true;

        if (!m_MissingAssetCallback || entry.logicalPath.empty())
            return false;

        if (!m_MissingAssetCallback(entry.logicalPath))
            return false;

        return !ResolveRuntimePathOnDisk(entry.runtimePath).empty();
    }
}
