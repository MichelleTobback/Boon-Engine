#pragma once
#include "AssetImporter.h"
#include "Asset/SpriteAtlasAsset.h"
#include "Core/ServiceLocator.h"
#include "Asset/AssetLibrary.h"
#include "Asset/TextureAsset.h"
#include <nlohmann/json.hpp>
#include <fstream>

namespace Boon
{
    class SpriteAtlasImporter : public AssetImporter
    {
    public:
        using AssetType = SpriteAtlasAsset;

        Asset* ImportFromFile(const std::string& filePath, const AssetMeta& meta) override
        {
            SpriteAtlasAsset* pResult = nullptr;
            std::shared_ptr<SpriteAtlas> pInstance = std::make_shared<SpriteAtlas>();

            std::ifstream file(filePath);

            if (!file)
                return pResult;

            nlohmann::json j;
            file >> j;

            AssetHandle handle{ meta.uuid };

            AssetLibrary& assets = ServiceLocator::Get<AssetLibrary>();
            AssetRef<Texture2DAsset> tex = assets.Import<Texture2DAsset>(j["texture"].get<std::string>());
            pInstance->SetTexture(tex);

            for (auto& [name, entry] : j["sprites"].items())
            {
                SpriteFrame uv;
                uv.UV = { entry["x"], entry["y"] };
                uv.Size = { entry["w"], entry["h"] };
                uv.FrameTime = entry["time"];
                pInstance->SetSpriteFrame(uv, entry["id"]);
            }

            for (auto& entry : j["clips"])
            {
                SpriteAnimClip clip;
                clip.Frames = entry["frames"].get<std::vector<int>>();
                clip.Speed = entry["speed"];
                clip.pAtlas = pInstance.get();
                pInstance->AddClip(clip);
            }

            pResult = new SpriteAtlasAsset(handle, pInstance);
            return pResult;
        }

        virtual std::vector<std::string> GetExtensions() const override
        {
            return { ".bsa" };
        }
    };
}
