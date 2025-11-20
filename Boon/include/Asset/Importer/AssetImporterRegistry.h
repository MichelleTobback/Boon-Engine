#pragma once
#include "AssetImporter.h"
#include "Asset/AssetTraits.h"
#include "Asset/AssetPack/AssetCache.h"
#include "Asset/AssetRegistry.h"

#include <unordered_map>
#include <memory>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>

namespace Boon
{
    class AssetImporterRegistry
    {
    public:
        template <typename T>
        struct Imported
        {
            Imported() = default;
            Imported(T* _asset, const AssetMeta& _meta)
                : asset(_asset), meta(_meta){ }

            T* asset;
            AssetMeta meta;
        };

        static AssetImporterRegistry& Get()
        {
            static AssetImporterRegistry inst;
            return inst;
        }

        template<typename TImporter>
        void RegisterImporter()
        {
            static_assert(std::is_base_of_v<Asset, TImporter::AssetType>);
            static_assert(std::is_base_of_v<AssetImporter, TImporter>);

            AssetType type = AssetTraits<TImporter::AssetType>::Type;
            m_Importers[type] = std::make_unique<TImporter>();

            std::vector<std::string> extensions = m_Importers[type]->GetExtensions();
            for (auto& ext : extensions)
            {
                m_ExtentionsToType[ext] = type;
            }
        }

        AssetImporter* GetImporter(AssetType type)
        {
            auto it = m_Importers.find(type);
            return (it != m_Importers.end()) ? it->second.get() : nullptr;
        }

        template<typename T>
        Imported<T> ImportAndLoad(const std::string& filepath)
        {
            AssetMeta meta = MetaFromFile(filepath);
            if (!meta.IsValid())
                return {};

            m_pRegistry->Add(meta);

            return Imported<T>( dynamic_cast<T*>(m_Importers[meta.type]->ImportFromFile(filepath, meta)), meta );
        }

        Imported<Asset> ImportAndLoad(const std::string& filepath)
        {
            AssetMeta meta = MetaFromFile(filepath);
            if (!meta.IsValid())
                return {};

            m_pRegistry->Add(meta);

            Asset* pAsset = m_pCache->Find<Asset>(meta.uuid);
            if (!pAsset)
            {
                pAsset = m_Importers[meta.type]->ImportFromFile(filepath, meta);
                m_pCache->Store(pAsset);
            }
            return Imported<Asset>(pAsset, meta);
        }

        bool HasExtension(const std::string& extension) const
        {
            return m_ExtentionsToType.find(extension) != m_ExtentionsToType.end();
        }

        AssetMeta LoadMeta(const std::string& filepath)
        {
            AssetMeta meta;
            if (std::ifstream inputFile(filepath); inputFile.is_open())
            {
                nlohmann::json j;
                inputFile >> j;

                meta.uuid = j["uuid"].get<uint32_t>();
                meta.type = (AssetType)j["type"].get<uint32_t>();
            }
        }

        void WriteMeta(const std::string& filepath, const AssetMeta& meta)
        {
            std::filesystem::path path{ filepath };
            auto it = m_ExtentionsToType.find(path.extension().string());
            if (it == m_ExtentionsToType.end())
                return;

            path.append(".meta");

            if (std::ofstream outputFile(path); outputFile.is_open())
            {
                nlohmann::json json{};
                json["uuid"] = (uint64_t)meta.uuid;
                json["type"] = meta.type;
                outputFile << json.dump(4);
                outputFile.close();
            }
        }

        AssetRegistry* GetRegistry() const { return m_pRegistry; }

    private:
        friend class AssetLibrary;
        AssetMeta MetaFromFile(const std::string& filepath)
        {
            AssetMeta meta;

            std::filesystem::path extension{ filepath };
            auto it = m_ExtentionsToType.find(extension.extension().string());
            if (it == m_ExtentionsToType.end())
                return meta;


            std::filesystem::path path{ filepath + std::string(".meta") };
            if (!std::filesystem::exists(path))
            {
                if (std::ofstream outputFile(path); outputFile.is_open())
                {
                    meta.uuid = UUID();
                    meta.type = m_ExtentionsToType[extension.extension().string()];

                    nlohmann::json json{};
                    json["uuid"] = (uint64_t)meta.uuid;
                    json["type"] = meta.type;
                    outputFile << json.dump(4);
                    outputFile.close();
                }
            }
            else if (std::ifstream inputFile(path); inputFile.is_open())
            {
                nlohmann::json j;
                inputFile >> j;

                meta.uuid = j["uuid"].get<uint64_t>();
                meta.type = (AssetType)j["type"].get<uint32_t>();
            }
            return meta;
        }

        std::unordered_map<AssetType, std::unique_ptr<AssetImporter>> m_Importers;
        std::unordered_map<std::string, AssetType> m_ExtentionsToType;
        AssetCache* m_pCache;
        AssetRegistry* m_pRegistry;
    };
}
