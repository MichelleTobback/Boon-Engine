#pragma once

#include "Assets/Importer/AssetImporter.h"

#include "Asset/AssetLibrary.h"
#include "Asset/AssetManifest.h"
#include "Asset/AssetTraits.h"
#include "Core/ServiceLocator.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <vector>

namespace Boon
{
    class AssetImporterRegistry
    {
    public:
        struct AssetRoot
        {
            std::filesystem::path sourceRoot{};
            std::filesystem::path runtimeRoot{};
            std::filesystem::path manifestPath{};
        };

        AssetImporterRegistry() = default;

        void BindToAssetLibrary(AssetLibrary& assets)
        {
            assets.BindMissingAssetCallback(
                [this](const std::filesystem::path& logicalPath)
                {
                    return Import(logicalPath).IsValid();
                });
        }

        size_t AddAssetRoot(
            const std::filesystem::path& sourceRoot,
            const std::filesystem::path& runtimeRoot,
            const std::filesystem::path& manifestPath = {})
        {
            AssetRoot root{};
            root.sourceRoot = sourceRoot.lexically_normal();
            root.runtimeRoot = runtimeRoot.lexically_normal();
            root.manifestPath = manifestPath.empty()
                ? (root.runtimeRoot / "AssetManifest.json").lexically_normal()
                : manifestPath.lexically_normal();

            m_Roots.push_back(root);
            return m_Roots.size() - 1;
        }

        void ClearAssetRoots()
        {
            m_Roots.clear();
        }

        const std::vector<AssetRoot>& GetAssetRoots() const
        {
            return m_Roots;
        }

        template<typename TImporter>
        void RegisterImporter()
        {
            static_assert(std::is_base_of_v<AssetImporter, TImporter>);
            static_assert(std::is_base_of_v<Asset, typename TImporter::AssetType>);

            const AssetType type = AssetTraits<typename TImporter::AssetType>::Type;
            auto importer = std::make_unique<TImporter>();

            for (const std::string& ext : importer->GetExtensions())
                m_ExtensionsToType[NormalizeExtension(ext)] = type;

            m_Importers[type] = std::move(importer);
        }

        bool HasExtension(const std::string& extension) const
        {
            return m_ExtensionsToType.find(NormalizeExtension(extension)) != m_ExtensionsToType.end();
        }

        AssetImporter* GetImporter(AssetType type)
        {
            auto it = m_Importers.find(type);
            return it == m_Importers.end() ? nullptr : it->second.get();
        }

        AssetMeta Import(const std::filesystem::path& sourceOrLogicalPath)
        {
            ResolvedImportPath resolved = ResolveImportPath(sourceOrLogicalPath);
            if (!resolved.Valid)
                return {};

            return Import(resolved);
        }

        AssetMeta ImportFromRoot(size_t rootIndex, const std::filesystem::path& sourcePath)
        {
            if (rootIndex >= m_Roots.size())
                return {};

            ResolvedImportPath resolved{};
            if (!ResolveWithinRoot(rootIndex, sourcePath, resolved))
                return {};

            return Import(resolved);
        }

        template<typename T>
        bool Export(const std::filesystem::path& filepath, AssetHandle asset)
        {
            AssetLibrary& assets = ServiceLocator::Get<AssetLibrary>();
            T* pAsset = asset.IsValid() ? assets.Resolve<T>(asset) : nullptr;
            AssetImporter* importer = GetImporter(AssetTraits<T>::Type);
            return importer && importer->ExportToFile(filepath, pAsset);
        }

        AssetMeta LoadMeta(const std::filesystem::path& filepath) const
        {
            AssetMeta meta{};
            const std::filesystem::path metaPath = GetMetaPath(filepath);

            std::ifstream inputFile(metaPath);
            if (!inputFile)
                return meta;

            nlohmann::json j{};
            inputFile >> j;

            meta.uuid = AssetHandle(j.value("uuid", uint64_t{ 0 }));
            meta.type = static_cast<AssetType>(j.value("type", static_cast<uint32_t>(AssetType::None)));
            meta.sourcePath = j.value("sourcePath", std::string{});
            meta.runtimePath = j.value("runtimePath", std::string{});

            if (j.contains("settings"))
                meta.settings = j["settings"].get<std::unordered_map<std::string, std::string>>();

            if (j.contains("dependencies"))
            {
                for (uint64_t dep : j["dependencies"].get<std::vector<uint64_t>>())
                    meta.dependencies.emplace_back(dep);
            }

            return meta;
        }

        void WriteMeta(const std::filesystem::path& filepath, const AssetMeta& meta) const
        {
            const std::filesystem::path metaPath = GetMetaPath(filepath);
            std::filesystem::create_directories(metaPath.parent_path());

            std::ofstream outputFile(metaPath, std::ios::trunc);
            if (!outputFile)
                return;

            nlohmann::json json{};
            json["uuid"] = static_cast<uint64_t>(meta.uuid);
            json["type"] = static_cast<uint32_t>(meta.type);
            json["sourcePath"] = meta.sourcePath.generic_string();
            json["runtimePath"] = meta.runtimePath.generic_string();
            json["settings"] = meta.settings;

            std::vector<uint64_t> dependencies;
            dependencies.reserve(meta.dependencies.size());
            for (AssetHandle handle : meta.dependencies)
                dependencies.push_back(static_cast<uint64_t>(handle));
            json["dependencies"] = dependencies;

            outputFile << json.dump(4);
        }

        static std::filesystem::path ToRuntimePath(const std::filesystem::path& logicalPath, UUID uuid)
        {
            std::filesystem::path runtimePath = "Assets" / std::filesystem::path(logicalPath.stem().string() + "_" + std::to_string(uuid) + ".basset");
            return runtimePath;
        }

    private:
        struct ResolvedImportPath
        {
            bool Valid = false;
            size_t RootIndex = 0;
            UUID uuid;
            std::filesystem::path SourcePath{};
            std::filesystem::path LogicalPath{};
            std::filesystem::path RuntimePath{};
            std::filesystem::path OutputPath{};
        };

        AssetMeta Import(const ResolvedImportPath& resolved)
        {
            AssetMeta meta = MetaFromFile(resolved);
            if (!meta.IsValid())
                return {};

            AssetImporter* importer = GetImporter(meta.type);
            if (!importer)
                return {};

            if (!importer->ImportToBAsset(resolved.SourcePath, resolved.OutputPath, meta))
                return {};

            WriteMeta(resolved.SourcePath, meta);

            m_MetaRootIndex[meta.uuid] = resolved.RootIndex;

            if (ServiceLocator::Has<AssetLibrary>())
                ServiceLocator::Get<AssetLibrary>().RegisterMeta(meta);

            SaveManifestForRoot(resolved.RootIndex);

            return meta;
        }

        AssetMeta LoadOrCreateMeta(const std::filesystem::path& path)
        {
            AssetMeta meta{};

            std::string ext = NormalizeExtension(path.extension().string());
            auto typeIt = m_ExtensionsToType.find(ext);
            if (typeIt == m_ExtensionsToType.end())
                return meta;

            const std::filesystem::path metaPath = GetMetaPath(path);
            if (std::filesystem::exists(metaPath))
                meta = LoadMeta(path);
            else
            {
                meta.uuid = UUID();
                meta.type = typeIt->second;
                meta.settings = {};
            }

            if (meta.type == AssetType::None)
                meta.type = typeIt->second;

            return meta;
        }

        AssetMeta MetaFromFile(const ResolvedImportPath& resolved)
        {
            AssetMeta meta{};

            std::string ext = NormalizeExtension(resolved.SourcePath.extension().string());
            auto typeIt = m_ExtensionsToType.find(ext);
            if (typeIt == m_ExtensionsToType.end())
                return meta;

            const std::filesystem::path metaPath = GetMetaPath(resolved.SourcePath);
            if (std::filesystem::exists(metaPath))
                meta = LoadMeta(resolved.SourcePath);
            else
            {
                meta.uuid = UUID();
                meta.type = typeIt->second;
                meta.settings = {};
            }

            if (meta.type == AssetType::None)
                meta.type = typeIt->second;

            meta.sourcePath = resolved.LogicalPath.lexically_normal();
            meta.runtimePath = resolved.RuntimePath.lexically_normal();

            return meta;
        }

        ResolvedImportPath ResolveImportPath(const std::filesystem::path& sourceOrLogicalPath) const
        {
            ResolvedImportPath resolved{};

            for (size_t i = 0; i < m_Roots.size(); ++i)
            {
                if (ResolveWithinRoot(i, sourceOrLogicalPath, resolved))
                    return resolved;
            }

            return {};
        }

        bool ResolveWithinRoot(
            size_t rootIndex,
            const std::filesystem::path& sourceOrLogicalPath,
            ResolvedImportPath& out) const
        {
            const AssetRoot& root = m_Roots[rootIndex];

            std::filesystem::path sourcePath;
            std::filesystem::path logicalPath;

            if (sourceOrLogicalPath.is_absolute())
            {
                sourcePath = sourceOrLogicalPath.lexically_normal();
                logicalPath = MakeRelativePath(sourcePath, root.sourceRoot);
            }
            else
            {
                logicalPath = sourceOrLogicalPath.lexically_normal();
                sourcePath = (root.sourceRoot / logicalPath).lexically_normal();
            }

            if (logicalPath.empty())
                return false;

            if (!std::filesystem::exists(sourcePath))
                return false;

            std::string logicalKey = AssetManifest::NormalizePathKey(logicalPath);
            if (logicalKey.starts_with(".."))
                return false;

            const AssetMeta existingMeta = LoadMeta(sourcePath);
            const UUID uuid = existingMeta.uuid.IsValid() ? existingMeta.uuid : UUID();

            out.Valid = true;
            out.RootIndex = rootIndex;
            out.uuid = uuid;
            out.SourcePath = sourcePath;
            out.LogicalPath = logicalPath.lexically_normal();
            out.RuntimePath = ToRuntimePath(out.LogicalPath, uuid);
            out.OutputPath = (root.runtimeRoot / out.RuntimePath).lexically_normal();

            return true;
        }

        static std::filesystem::path MakeRelativePath(
            const std::filesystem::path& path,
            const std::filesystem::path& root)
        {
            std::error_code ec;
            std::filesystem::path relative = std::filesystem::relative(path, root, ec);

            if (!ec && !relative.empty())
            {
                std::string rel = relative.generic_string();
                if (!rel.starts_with(".."))
                    return relative.lexically_normal();
            }

            return {};
        }

        void SaveManifestForRoot(size_t rootIndex) const
        {
            if (!ServiceLocator::Has<AssetLibrary>())
                return;

            if (rootIndex >= m_Roots.size())
                return;

            AssetManifest manifest{};

            for (const auto& [handle, meta] : ServiceLocator::Get<AssetLibrary>().GetRegistry().GetAll())
            {
                auto rootIt = m_MetaRootIndex.find(handle);
                if (rootIt != m_MetaRootIndex.end() && rootIt->second != rootIndex)
                    continue;

                manifest.Add(meta);
            }

            manifest.SaveToFile(m_Roots[rootIndex].manifestPath);
        }

        static std::filesystem::path GetMetaPath(const std::filesystem::path& filepath)
        {
            return std::filesystem::path(filepath.string() + ".meta");
        }

        static std::string NormalizeExtension(std::string ext)
        {
            if (!ext.empty() && ext[0] != '.')
                ext.insert(ext.begin(), '.');

            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            return ext;
        }

    private:
        std::unordered_map<AssetType, std::unique_ptr<AssetImporter>> m_Importers;
        std::unordered_map<std::string, AssetType> m_ExtensionsToType;
        std::unordered_map<AssetHandle, size_t> m_MetaRootIndex;
        std::vector<AssetRoot> m_Roots;
    };
}
