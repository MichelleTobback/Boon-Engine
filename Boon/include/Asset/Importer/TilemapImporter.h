#pragma once
#include "AssetImporter.h"
#include "Asset/TilemapAsset.h"
#include "Asset/SpriteAtlasAsset.h"
#include "Core/ServiceLocator.h"
#include "Asset/AssetLibrary.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace Boon
{
    class TilemapImporter : public AssetImporter
    {
    public:
        using AssetType = TilemapAsset;

        Asset* ImportFromFile(const std::string& filePath, const AssetMeta& meta) override
        {
            TilemapAsset* pResult = nullptr;

            std::ifstream file(filePath);

            if (!file)
                return pResult;

            nlohmann::json j;
            file >> j;

            AssetHandle handle{ meta.uuid };

            int chunksX = j["chunksX"].get<int>();
            int chunksY = j["chunksY"].get<int>();
            int chunksSize = j["chunksSize"].get<int>();
            std::shared_ptr<Tilemap> pInstance = std::make_shared<Tilemap>(chunksX, chunksY, chunksSize);

            AssetLibrary& assets = ServiceLocator::Get<AssetLibrary>();
            AssetRef<SpriteAtlasAsset> spriteAtlas = assets.Import<SpriteAtlasAsset>(j["atlas"].get<std::string>());
            pInstance->SetAtlas(spriteAtlas);

            for (auto& entry : j["tiles"])
            {
                int x = entry["x"].get<int>();
                int y = entry["y"].get<int>();
                int sprite = entry["sprite"].get<int>();

                pInstance->SetTile(x, y, sprite);
            }

            pResult = new TilemapAsset(handle, pInstance);
            return pResult;
        }

        bool ExportToFile(const std::string& filePath, Asset* asset)
        {
            TilemapAsset* pAsset = dynamic_cast<TilemapAsset*>(asset);

            if (!pAsset)
                return false;

            std::string texPath = "";
            {
                std::ifstream in(filePath);

                nlohmann::json j;
                in >> j;

                texPath = j["atlas"].get<std::string>();
            }

            std::ofstream file(filePath);
            if (!file)
                return false;

            nlohmann::json j;

            std::shared_ptr<Tilemap> pTilemap = pAsset->GetInstance();
            if (!pTilemap)
                return false;

            // Basic layout info
            j["chunksX"] = pTilemap->GetChunksX();
            j["chunksY"] = pTilemap->GetChunksY();
            j["chunksSize"] = pTilemap->GetChunkSize();

            // Atlas path (same as importer expects)
            AssetRef<SpriteAtlasAsset> atlas = pTilemap->GetAtlas();
            if (atlas.IsValid())
                j["atlas"] = texPath;
            else
                j["atlas"] = "";

            // Save tiles
            nlohmann::json tileArray = nlohmann::json::array();

            int width = pTilemap->GetChunksX() * pTilemap->GetChunkSize();
            int height = pTilemap->GetChunksY() * pTilemap->GetChunkSize();

            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    int sprite = pTilemap->GetTile(x, y);
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

            // Write JSON
            file << j.dump(4); // pretty print
            return true;
        }

        virtual std::vector<std::string> GetExtensions() const override
        {
            return { ".btm" };
        }
    };
}
