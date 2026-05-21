#pragma once

#include "Assets/Importer/AssetImporter.h"
#include "Asset/AssetLibrary.h"
#include "Asset/Runtime/BAssetFile.h"
#include "Asset/SpriteAtlasAsset.h"
#include "Asset/TilemapAsset.h"
#include "Core/ServiceLocator.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace Boon
{
    class TilemapImporter : public AssetImporter
    {
    public:
        using AssetType = TilemapAsset;

        bool ImportToBAsset(
            const std::filesystem::path& sourcePath, 
            const std::filesystem::path& exportPath, 
            const AssetMeta& meta) override
        {
            std::ifstream file(sourcePath);
            if (!file)
                return false;

            nlohmann::json j;
            file >> j;

            const int chunksX = j["chunksX"].get<int>();
            const int chunksY = j["chunksY"].get<int>();
            const int chunkSize = j["chunksSize"].get<int>();

            std::shared_ptr<Tilemap> instance = std::make_shared<Tilemap>(chunksX, chunksY, chunkSize);

            AssetLibrary& assets = ServiceLocator::Get<AssetLibrary>();
            AssetRef<SpriteAtlasAsset> atlas = assets.Load<SpriteAtlasAsset>(j.value("atlas", std::string{}));
            instance->SetAtlas(atlas);

            for (const auto& entry : j["tiles"])
            {
                const int x = entry["x"].get<int>();
                const int y = entry["y"].get<int>();
                const int sprite = entry["sprite"].get<int>();
                instance->SetTile(x, y, sprite);
            }

            TilemapAsset asset(meta.uuid, instance);
            Buffer payload = AssetSerializer<TilemapAsset>::Serialize(&asset);
            return BAssetFile::Write(exportPath, meta, payload);
        }

        bool ExportToFile(const std::filesystem::path& filePath, Asset* asset) override
        {
            TilemapAsset* tilemapAsset = dynamic_cast<TilemapAsset*>(asset);
            if (!tilemapAsset)
                return false;

            std::string atlasPath;
            {
                std::ifstream in(filePath);
                if (in)
                {
                    nlohmann::json existing;
                    in >> existing;
                    atlasPath = existing.value("atlas", std::string{});
                }
            }

            std::shared_ptr<Tilemap> tilemap = tilemapAsset->GetInstance();
            if (!tilemap)
                return false;

            nlohmann::json j;
            j["chunksX"] = tilemap->GetChunksX();
            j["chunksY"] = tilemap->GetChunksY();
            j["chunksSize"] = tilemap->GetChunkSize();
            j["atlas"] = atlasPath;

            nlohmann::json tileArray = nlohmann::json::array();
            const int width = tilemap->GetChunksX() * tilemap->GetChunkSize();
            const int height = tilemap->GetChunksY() * tilemap->GetChunkSize();

            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    const int sprite = tilemap->GetTile(x, y);
                    if (sprite >= 0)
                    {
                        tileArray.push_back({
                            { "x", x },
                            { "y", y },
                            { "sprite", sprite }
                        });
                    }
                }
            }

            j["tiles"] = tileArray;

            std::filesystem::create_directories(filePath.parent_path());
            std::ofstream file(filePath);
            if (!file)
                return false;

            file << j.dump(4);
            return true;
        }

        std::vector<std::string> GetExtensions() const override
        {
            return { ".btm" };
        }
    };
}
