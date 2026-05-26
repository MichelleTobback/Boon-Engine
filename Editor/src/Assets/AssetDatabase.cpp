#include "Assets/AssetDatabase.h"
#include "Asset/Assets.h"

#include <algorithm>
#include <system_error>

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

    namespace
    {
        static std::filesystem::path MetaPath(const std::filesystem::path& path)
        {
            return std::filesystem::path(path.string() + ".meta");
        }

        static bool IsInsideRoot(
            const std::filesystem::path& path,
            const std::filesystem::path& root,
            std::filesystem::path& outRelative)
        {
            std::error_code ec;
            outRelative = std::filesystem::relative(path, root, ec);

            if (ec || outRelative.empty())
                return false;

            const std::string rel = outRelative.generic_string();
            return !rel.starts_with("..");
        }

        static std::filesystem::path MakeUniquePath(std::filesystem::path path)
        {
            if (!std::filesystem::exists(path))
                return path;

            const auto parent = path.parent_path();
            const auto stem = path.stem().string();
            const auto ext = path.extension().string();

            int index = 1;

            do
            {
                path = parent / (stem + "_" + std::to_string(index) + ext);
                ++index;
            } while (std::filesystem::exists(path));

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
        
    }

    void AssetDatabase::Init(AssetLibrary& assetLib)
    {
        m_pAssetLib = &assetLib;
        m_DefaultTextures[AssetType::Texture] = assetLib.Load<Texture2DAsset>("Icons/Assets/texture_icon.png");
        m_DefaultTextures[AssetType::SpriteAtlas] = assetLib.Load<Texture2DAsset>("Icons/Assets/sprite_atlas_icon.png");
        m_DefaultTextures[AssetType::Tilemap] = assetLib.Load<Texture2DAsset>("Icons/Assets/sprite_atlas_icon.png");
        m_DefaultTextures[AssetType::Shader] = assetLib.Load<Texture2DAsset>("Icons/Assets/shader_icon.png");
        m_DefaultTextures[AssetType::Scene] = assetLib.Load<Texture2DAsset>("Icons/Assets/scene_icon.png");
    }

    void AssetDatabase::RegisterAsset(const std::string& path, AssetHandle handle, int rootIndex)
    {
        std::string cleanPath = Utils::NormalizePath(path);
        m_HandleToPath[handle].Path = cleanPath;
        m_HandleToPath[handle].Root = rootIndex;
        m_PathToHandle[cleanPath] = handle;
        m_Dirty = true;
    }

    AssetHandle AssetDatabase::GetHandle(const std::string& path) const
    {
        if (auto it = m_PathToHandle.find(Utils::NormalizePath(path)); it != m_PathToHandle.end())
            return it->second;
        return {};
    }

    const std::filesystem::path& AssetDatabase::GetPath(AssetHandle handle) const
    {
        static const std::filesystem::path empty{};

        if (auto it = m_HandleToPath.find(handle); it != m_HandleToPath.end())
            return it->second.Path;

        return empty;
    }

    void AssetDatabase::Remove(AssetHandle asset)
    {
        if (!asset.IsValid())
            return;

        auto it = m_HandleToPath.find(asset);
        if (it == m_HandleToPath.end())
            return;

        const Path storedPath = it->second;

        const std::filesystem::path logicalPath = storedPath.Path.lexically_normal();
        const std::filesystem::path sourcePath = storedPath.SourcePath().lexically_normal();

        AssetMeta meta{};

        if (std::filesystem::exists(MetaPath(sourcePath)))
            meta = ServiceLocator::Get<AssetImporterRegistry>().LoadMeta(sourcePath);
        else if (const AssetMeta* registeredMeta = m_pAssetLib->GetMeta(asset))
            meta = *registeredMeta;

        std::error_code ec;

        if (std::filesystem::exists(sourcePath))
            std::filesystem::remove(sourcePath, ec);

        if (std::filesystem::exists(MetaPath(sourcePath)))
            std::filesystem::remove(MetaPath(sourcePath), ec);

        if (meta.IsValid() && !meta.runtimePath.empty())
        {
            const auto& roots = ServiceLocator::Get<AssetImporterRegistry>().GetAssetRoots();

            if (storedPath.Root >= 0 && storedPath.Root < static_cast<int>(roots.size()))
            {
                const std::filesystem::path runtimePath =
                    (roots[storedPath.Root].runtimeRoot / meta.runtimePath).lexically_normal();

                if (std::filesystem::exists(runtimePath))
                    std::filesystem::remove(runtimePath, ec);
            }
        }

        m_PathToHandle.erase(Utils::NormalizePath(logicalPath.generic_string()));
        m_HandleToPath.erase(asset);

        AssetLibrary& assets = *m_pAssetLib;
        assets.GetRegistry().Remove(asset);
        assets.GetManifest().Remove(asset);
        assets.ClearCache();

        const auto& roots = ServiceLocator::Get<AssetImporterRegistry>().GetAssetRoots();

        if (storedPath.Root >= 0 && storedPath.Root < static_cast<int>(roots.size()))
        {
            AssetManifest manifest{};

            for (const auto& [handle, path] : m_HandleToPath)
            {
                if (path.Root != storedPath.Root)
                    continue;

                const AssetMeta* entryMeta = assets.GetMeta(handle);
                if (entryMeta && entryMeta->IsValid())
                    manifest.Add(*entryMeta);
            }

            manifest.SaveToFile(roots[storedPath.Root].manifestPath);
        }

        m_Dirty = true;
    }


    void AssetDatabase::RemoveRecursively(const std::filesystem::path& folder)
    {
        std::vector<AssetHandle> toRemove;

        const auto folderPath =
            folder.lexically_normal().generic_string();

        for (const auto& [handle, path] : m_HandleToPath)
        {
            const auto assetPath =
                path.SourcePath().lexically_normal().generic_string();

            if (assetPath.starts_with(folderPath))
                toRemove.push_back(handle);
        }

        for (AssetHandle handle : toRemove)
            Remove(handle);

        std::error_code ec;
        std::filesystem::remove_all(folder, ec);

        m_Dirty = true;
    }

    AssetHandle AssetDatabase::Move(
        const std::filesystem::path& from,
        const std::filesystem::path& to)
    {
        const std::string normalized =
            Utils::NormalizePath(from.generic_string());

        AssetHandle handle = GetHandle(normalized);

        if (!handle.IsValid())
        {
            for (const auto& [candidateHandle, candidatePath] : m_HandleToPath)
            {
                if (Utils::NormalizePath(candidatePath.Path.generic_string()) == normalized ||
                    Utils::NormalizePath(candidatePath.SourcePath().generic_string()) == normalized ||
                    Utils::NormalizePath(candidatePath.FullPath().generic_string()) == normalized)
                {
                    handle = candidateHandle;
                    break;
                }
            }
        }

        if (!handle.IsValid())
            return {};

        auto it = m_HandleToPath.find(handle);

        if (it == m_HandleToPath.end())
            return {};

        Path& record = it->second;

        std::filesystem::path oldSource =
            record.SourcePath().lexically_normal();

        std::filesystem::path newSource =
            to.lexically_normal();

        const auto& roots =
            ServiceLocator::Get<AssetImporterRegistry>().GetAssetRoots();

        if (record.Root < 0 ||
            record.Root >= static_cast<int>(roots.size()))
        {
            return {};
        }

        if (std::filesystem::exists(newSource) &&
            std::filesystem::is_directory(newSource))
        {
            newSource /= oldSource.filename();
        }

        std::filesystem::create_directories(
            newSource.parent_path());
        AssetDatabase::Get().MarkDirty();

        std::error_code ec;

        std::filesystem::rename(oldSource, newSource, ec);

        if (ec)
            return {};

        std::filesystem::path oldMeta = oldSource;
        oldMeta += ".meta";

        std::filesystem::path newMeta = newSource;
        newMeta += ".meta";

        if (std::filesystem::exists(oldMeta))
            std::filesystem::rename(oldMeta, newMeta, ec);

        std::filesystem::path newLogical = std::filesystem::relative(newSource, roots[record.Root].sourceRoot);

        AssetMeta meta = ServiceLocator::Get<AssetImporterRegistry>().LoadMeta(newSource);

        if (meta.IsValid())
        {
            meta.sourcePath = newLogical.lexically_normal();

            ServiceLocator::Get<AssetImporterRegistry>().WriteMeta(newSource, meta);
        }

        m_PathToHandle.erase(Utils::NormalizePath(record.Path.generic_string()));

        record.Path = newLogical.lexically_normal();

        m_PathToHandle[Utils::NormalizePath(record.Path.generic_string())] = handle;

        RebuildAndSaveManifestForRoot(record.Root);

        m_Dirty = true;

        return handle;
    }

    void AssetDatabase::MoveRecursively(const std::filesystem::path& from, const std::filesystem::path& to)
    {
        const std::filesystem::path oldFolder = from.lexically_normal();

        std::filesystem::path newFolder = to.lexically_normal();

        if (std::filesystem::exists(newFolder) && std::filesystem::is_directory(newFolder))
        {
            newFolder /= oldFolder.filename();
        }

        std::filesystem::create_directories(newFolder.parent_path());
        AssetDatabase::Get().MarkDirty();

        std::error_code ec;

        std::filesystem::rename(oldFolder, newFolder, ec);

        if (ec)
            return;

        const auto& roots = ServiceLocator::Get<AssetImporterRegistry>().GetAssetRoots();

        for (auto& [handle, record] : m_HandleToPath)
        {
            std::filesystem::path source = record.SourcePath().lexically_normal();

            std::filesystem::path rel = std::filesystem::relative(source, oldFolder, ec);

            if (ec || rel.empty() || rel.generic_string().starts_with(".."))
            {
                continue;
            }

            std::filesystem::path movedSource = newFolder / rel;

            std::filesystem::path newLogical = std::filesystem::relative(movedSource, roots[record.Root].sourceRoot);

            m_PathToHandle.erase(Utils::NormalizePath(record.Path.generic_string()));

            record.Path = newLogical.lexically_normal();

            m_PathToHandle[Utils::NormalizePath(record.Path.generic_string())] = handle;

            AssetMeta meta = ServiceLocator::Get<AssetImporterRegistry>().LoadMeta(movedSource);

            if (meta.IsValid())
            {
                meta.sourcePath = record.Path;

                ServiceLocator::Get<AssetImporterRegistry>().WriteMeta(movedSource, meta);
            }
        }

        for (int i = 0; i < static_cast<int>(roots.size()); ++i)
            RebuildAndSaveManifestForRoot(i);

        m_Dirty = true;
    }

    AssetHandle AssetDatabase::Copy(AssetHandle asset, const std::filesystem::path& to)
    {
        if (!asset.IsValid())
            return {};

        auto it = m_HandleToPath.find(asset);
        if (it == m_HandleToPath.end())
            return {};

        const Path sourceRecord = it->second;

        const std::filesystem::path sourcePath =
            sourceRecord.SourcePath().lexically_normal();

        if (!std::filesystem::exists(sourcePath))
            return {};

        const auto& roots =
            ServiceLocator::Get<AssetImporterRegistry>().GetAssetRoots();

        if (sourceRecord.Root < 0 || sourceRecord.Root >= static_cast<int>(roots.size()))
            return {};

        int targetRootIndex = sourceRecord.Root;
        std::filesystem::path targetSourcePath;

        if (to.is_absolute())
        {
            std::filesystem::path relative;

            bool foundRoot = false;

            for (size_t i = 0; i < roots.size(); ++i)
            {
                if (IsInsideRoot(to.lexically_normal(), roots[i].sourceRoot, relative))
                {
                    targetRootIndex = static_cast<int>(i);
                    foundRoot = true;
                    break;
                }
            }

            if (!foundRoot)
                return {};

            targetSourcePath = to.lexically_normal();
        }
        else
        {
            targetSourcePath =
                (roots[targetRootIndex].sourceRoot / to).lexically_normal();
        }

        if (std::filesystem::exists(targetSourcePath) &&
            std::filesystem::is_directory(targetSourcePath))
        {
            targetSourcePath /= sourcePath.filename();
        }

        if (!targetSourcePath.has_extension())
            targetSourcePath.replace_extension(sourcePath.extension());

        targetSourcePath = MakeUniquePath(targetSourcePath);

        std::filesystem::create_directories(targetSourcePath.parent_path());
        AssetDatabase::Get().MarkDirty();

        std::error_code ec;
        std::filesystem::copy_file(sourcePath, targetSourcePath, ec);

        if (ec)
            return {};

        AssetMeta sourceMeta =
            ServiceLocator::Get<AssetImporterRegistry>().LoadMeta(sourcePath);

        if (sourceMeta.IsValid())
        {
            std::filesystem::path logicalPath;

            if (!IsInsideRoot(targetSourcePath, roots[targetRootIndex].sourceRoot, logicalPath))
                return {};

            AssetMeta copiedMeta = sourceMeta;
            copiedMeta.uuid = UUID();
            copiedMeta.sourcePath = logicalPath.lexically_normal();
            copiedMeta.runtimePath =
                AssetImporterRegistry::ToRuntimePath(copiedMeta.sourcePath, copiedMeta.uuid);

            ServiceLocator::Get<AssetImporterRegistry>().WriteMeta(targetSourcePath, copiedMeta);
        }

        AssetMeta imported =
            ServiceLocator::Get<AssetImporterRegistry>().ImportFromRoot(
                static_cast<size_t>(targetRootIndex),
                targetSourcePath);

        if (!imported.IsValid())
            return {};

        RegisterAsset(
            imported.sourcePath.generic_string(),
            imported.uuid,
            targetRootIndex);

        m_Dirty = true;
        return imported.uuid;
    }

    void AssetDatabase::Remove(const std::filesystem::path& path)
    {
        const std::string normalized = Utils::NormalizePath(path.generic_string());

        AssetHandle handle = GetHandle(normalized);

        if (!handle.IsValid())
        {
            const std::filesystem::path query = path.lexically_normal();

            for (const auto& [candidateHandle, candidatePath] : m_HandleToPath)
            {
                if (Utils::NormalizePath(candidatePath.Path.generic_string()) == normalized ||
                    Utils::NormalizePath(candidatePath.SourcePath().generic_string()) == normalized ||
                    Utils::NormalizePath(candidatePath.FullPath().generic_string()) == normalized)
                {
                    handle = candidateHandle;
                    break;
                }
            }
        }

        Remove(handle);
    }

    AssetHandle AssetDatabase::Copy(
        const std::filesystem::path& from,
        const std::filesystem::path& to)
    {
        const std::string normalized = Utils::NormalizePath(from.generic_string());

        AssetHandle handle = GetHandle(normalized);

        if (!handle.IsValid())
        {
            for (const auto& [candidateHandle, candidatePath] : m_HandleToPath)
            {
                if (Utils::NormalizePath(candidatePath.Path.generic_string()) == normalized ||
                    Utils::NormalizePath(candidatePath.SourcePath().generic_string()) == normalized ||
                    Utils::NormalizePath(candidatePath.FullPath().generic_string()) == normalized)
                {
                    handle = candidateHandle;
                    break;
                }
            }
        }

        return Copy(handle, to);
    }

    std::vector<AssetHandle> AssetDatabase::CopyRecursively(
        const std::filesystem::path& from,
        const std::filesystem::path& to)
    {
        std::vector<AssetHandle> copied;

        if (!std::filesystem::exists(from) || !std::filesystem::is_directory(from))
            return copied;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(from))
        {
            if (!entry.is_regular_file())
                continue;

            const auto src = entry.path();

            if (src.extension() == ".meta")
                continue;

            const auto rel = std::filesystem::relative(src, from);
            const auto dst = to / rel;

            AssetHandle newHandle = Copy(src, dst);

            if (newHandle.IsValid())
                copied.push_back(newHandle);
        }

        m_Dirty = true;
        return copied;
    }
    AssetHandle AssetDatabase::Rename(
        const std::filesystem::path& from,
        const std::string& newName)
    {
        const std::string normalized =
            Utils::NormalizePath(from.generic_string());

        AssetHandle handle = GetHandle(normalized);

        if (!handle.IsValid())
        {
            for (const auto& [candidateHandle, candidatePath] : m_HandleToPath)
            {
                if (Utils::NormalizePath(candidatePath.Path.generic_string()) == normalized ||
                    Utils::NormalizePath(candidatePath.SourcePath().generic_string()) == normalized ||
                    Utils::NormalizePath(candidatePath.FullPath().generic_string()) == normalized)
                {
                    handle = candidateHandle;
                    break;
                }
            }
        }

        if (!handle.IsValid())
            return {};

        auto it = m_HandleToPath.find(handle);

        if (it == m_HandleToPath.end())
            return {};

        Path& record = it->second;

        std::filesystem::path oldSource =
            record.SourcePath().lexically_normal();

        if (!std::filesystem::exists(oldSource))
            return {};

        std::filesystem::path newSource =
            oldSource.parent_path() / newName;

        if (!newSource.has_extension())
            newSource.replace_extension(oldSource.extension());

        if (std::filesystem::exists(newSource))
            return {};

        std::filesystem::path oldMeta = oldSource;
        oldMeta += ".meta";

        std::filesystem::path newMeta = newSource;
        newMeta += ".meta";

        std::error_code ec;

        std::filesystem::rename(oldSource, newSource, ec);

        if (ec)
            return {};

        if (std::filesystem::exists(oldMeta))
            std::filesystem::rename(oldMeta, newMeta, ec);

        const auto& roots =
            ServiceLocator::Get<AssetImporterRegistry>().GetAssetRoots();

        if (record.Root < 0 ||
            record.Root >= static_cast<int>(roots.size()))
        {
            return {};
        }

        std::filesystem::path newLogical =
            std::filesystem::relative(
                newSource,
                roots[record.Root].sourceRoot);

        AssetMeta meta =
            ServiceLocator::Get<AssetImporterRegistry>().LoadMeta(newSource);

        if (meta.IsValid())
        {
            meta.sourcePath =
                newLogical.lexically_normal();

            ServiceLocator::Get<AssetImporterRegistry>()
                .WriteMeta(newSource, meta);
        }

        m_PathToHandle.erase(
            Utils::NormalizePath(record.Path.generic_string()));

        record.Path =
            newLogical.lexically_normal();

        m_PathToHandle[
            Utils::NormalizePath(record.Path.generic_string())] = handle;

        RebuildAndSaveManifestForRoot(record.Root);

        m_Dirty = true;

        return handle;
    }


    AssetHandle AssetDatabase::Rename(AssetHandle asset, const std::string& newName)
    {
        auto it = m_HandleToPath.find(asset);
        if (it == m_HandleToPath.end())
            return {};

        Path oldRecord = it->second;

        std::filesystem::path oldSource = oldRecord.SourcePath();
        std::filesystem::path newSource = oldSource.parent_path() / newName;

        if (!newSource.has_extension())
            newSource.replace_extension(oldSource.extension());

        std::filesystem::rename(oldSource, newSource);

        std::filesystem::path oldMeta = oldSource;
        oldMeta += ".meta";

        std::filesystem::path newMeta = newSource;
        newMeta += ".meta";

        if (std::filesystem::exists(oldMeta))
            std::filesystem::rename(oldMeta, newMeta);

        AssetMeta meta = ServiceLocator::Get<AssetImporterRegistry>().LoadMeta(newSource);

        std::filesystem::path logicalPath =
            std::filesystem::relative(
                newSource,
                ServiceLocator::Get<AssetImporterRegistry>()
                .GetAssetRoots()[oldRecord.Root]
                .sourceRoot);

        meta.sourcePath = logicalPath.lexically_normal();

        ServiceLocator::Get<AssetImporterRegistry>().WriteMeta(newSource, meta);

        m_PathToHandle.erase(Utils::NormalizePath(oldRecord.Path.generic_string()));

        oldRecord.Path = meta.sourcePath;
        m_HandleToPath[asset] = oldRecord;

        m_PathToHandle[Utils::NormalizePath(meta.sourcePath.generic_string())] = asset;

        m_Dirty = true;

        RebuildAndSaveManifestForRoot(oldRecord.Root);

        return asset;
    }

    void AssetDatabase::RenameFolder(
        const std::filesystem::path& from,
        const std::string& newName)
    {
        std::filesystem::path oldFolder = from.lexically_normal();
        std::filesystem::path newFolder = oldFolder.parent_path() / newName;

        std::filesystem::rename(oldFolder, newFolder);

        const auto& roots =
            ServiceLocator::Get<AssetImporterRegistry>().GetAssetRoots();

        for (auto& [handle, record] : m_HandleToPath)
        {
            std::filesystem::path sourcePath =
                record.SourcePath().lexically_normal();

            std::error_code ec;
            std::filesystem::path relToOld =
                std::filesystem::relative(sourcePath, oldFolder, ec);

            if (ec || relToOld.empty() || relToOld.generic_string().starts_with(".."))
                continue;

            std::filesystem::path newSourcePath =
                newFolder / relToOld;

            std::filesystem::path newLogical =
                std::filesystem::relative(
                    newSourcePath,
                    roots[record.Root].sourceRoot);

            m_PathToHandle.erase(
                Utils::NormalizePath(record.Path.generic_string()));

            record.Path = newLogical.lexically_normal();

            m_PathToHandle[
                Utils::NormalizePath(record.Path.generic_string())] = handle;

            AssetMeta meta =
                ServiceLocator::Get<AssetImporterRegistry>().LoadMeta(newSourcePath);

            if (meta.IsValid())
            {
                meta.sourcePath = record.Path;
                ServiceLocator::Get<AssetImporterRegistry>().WriteMeta(newSourcePath, meta);
            }
        }

        m_Dirty = true;

        for (int i = 0; i < static_cast<int>(roots.size()); ++i)
            RebuildAndSaveManifestForRoot(i);
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
            fn(handle, path.Path.string());
        }
    }

    AssetRef<Texture2DAsset> AssetDatabase::GetThumbnail(AssetHandle handle) const
    {
        const AssetMeta* meta = m_pAssetLib->GetMeta(handle);
        if (!meta)
            return AssetRef<Texture2DAsset>();

        if (meta->type == AssetType::Texture)
        {
            const std::filesystem::path& sourcePath = GetPath(handle);
            if (!sourcePath.empty())
                return m_pAssetLib->Load<Texture2DAsset>(sourcePath);

            return AssetRef<Texture2DAsset>();
        }

        auto loadDefault = [this](AssetType type, const std::string& path) -> AssetRef<Texture2DAsset>
            {
                auto it = m_DefaultTextures.find(type);
                if (it != m_DefaultTextures.end() && it->second.IsValid())
                    return it->second;

                AssetRef<Texture2DAsset> ref = m_pAssetLib->Load<Texture2DAsset>(path);
                m_DefaultTextures[type] = ref;
                return ref;
            };

        switch (meta->type)
        {
        case AssetType::SpriteAtlas:
            return loadDefault(AssetType::SpriteAtlas, "Icons/Assets/sprite_atlas_icon.png");
        case AssetType::Tilemap:
            return loadDefault(AssetType::Tilemap, "Icons/Assets/sprite_atlas_icon.png");
        case AssetType::Shader:
            return loadDefault(AssetType::Shader, "Icons/Assets/shader_icon.png");
        case AssetType::Scene:
            return loadDefault(AssetType::Scene, "Icons/Assets/scene_icon.png");
        default:
            return AssetRef<Texture2DAsset>();
        }
    }

    void AssetDatabase::RebuildAndSaveManifestForRoot(int rootIndex)
    {
        const auto& roots =
            ServiceLocator::Get<AssetImporterRegistry>().GetAssetRoots();

        if (rootIndex < 0 ||
            rootIndex >= static_cast<int>(roots.size()))
        {
            return;
        }

        AssetManifest manifest;

        AssetLibrary& assets = *m_pAssetLib;

        for (const auto& [handle, path] : m_HandleToPath)
        {
            if (path.Root != rootIndex)
                continue;

            const AssetMeta* meta =
                assets.GetMeta(handle);

            if (!meta || !meta->IsValid())
                continue;

            manifest.Add(*meta);
        }

        manifest.SaveToFile(
            roots[rootIndex].manifestPath);
    }
}
