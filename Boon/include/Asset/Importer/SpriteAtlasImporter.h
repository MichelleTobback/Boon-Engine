#pragma once
#include "AssetImporter.h"
#include "Asset/SpriteAtlasAsset.h"
#include "Core/ServiceLocator.h"
#include "Asset/AssetLibrary.h"
#include "Asset/TextureAsset.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <unordered_map>

namespace Boon
{
    class SpriteAtlasImporter : public AssetImporter
    {
    public:
        using AssetType = SpriteAtlasAsset;

        // --------------------------------------------------------------------
        // IMPORT (Supports old .bsa and new format)
        // --------------------------------------------------------------------
        Asset* ImportFromFile(const std::string& filePath, const AssetMeta& meta) override
        {
            SpriteAtlasAsset* pResult = nullptr;
            std::shared_ptr<SpriteAtlas> pInstance = std::make_shared<SpriteAtlas>();

            std::ifstream file(filePath);
            if (!file)
                return nullptr;

            nlohmann::json j;
            file >> j;

            AssetHandle handle{ meta.uuid };

            AssetLibrary& assets = ServiceLocator::Get<AssetLibrary>();
            std::string texPath = j["texture"].get<std::string>();
            if (!texPath.empty())
            {
                AssetRef<Texture2DAsset> tex = assets.Import<Texture2DAsset>(texPath);
                pInstance->SetTexture(tex);
            }

            // ---------------------------------------
            // FRAME IMPORT (backward compatible)
            // ---------------------------------------
            std::unordered_map<int, int> oldToNew;

            if (j.contains("sprites"))
            {
                nlohmann::json spritesJson = j["sprites"];
                auto it = spritesJson.begin();

                for (; it != spritesJson.end(); ++it)
                {
                    const std::string key = it.key();
                    const nlohmann::json& entry = it.value();

                    SpriteFrame uv;
                    uv.UV.x = entry["x"];
                    uv.UV.y = entry["y"];
                    uv.Size.x = entry["w"];
                    uv.Size.y = entry["h"];
                    uv.FrameTime = entry.contains("time") ? entry["time"].get<float>() : 0.0f;

                    int newId = pInstance->AddSpriteFrame(uv);

                    // old format included explicit "id"
                    if (entry.contains("id"))
                    {
                        int oldId = entry["id"].get<int>();
                        oldToNew[oldId] = newId;
                    }
                    else
                    {
                        // new format: key itself is the stable id
                        int implicitOld = std::stoi(key);
                        oldToNew[implicitOld] = newId;
                    }
                }
            }

            // ---------------------------------------
            // CLIP IMPORT
            // ---------------------------------------
            if (j.contains("clips"))
            {
                const nlohmann::json& clipsJson = j["clips"];
                for (size_t i = 0; i < clipsJson.size(); i++)
                {
                    const nlohmann::json& entry = clipsJson[i];

                    SpriteAnimClip clip;
                    clip.Speed = entry.contains("speed") ? entry["speed"].get<float>() : 1.0f;
                    clip.pAtlas = pInstance.get();

                    std::vector<int> frames = entry["frames"].get<std::vector<int>>();
                    clip.Frames.reserve(frames.size());

                    for (size_t f = 0; f < frames.size(); f++)
                    {
                        int oldId = frames[f];
                        if (oldToNew.count(oldId))
                            clip.Frames.push_back(oldToNew[oldId]);
                        else
                            clip.Frames.push_back(oldId);
                    }

                    pInstance->AddClip(clip);
                }
            }

            pResult = new SpriteAtlasAsset(handle, pInstance);
            return pResult;
        }

        // --------------------------------------------------------------------
        // EXPORT (New format only)
        // --------------------------------------------------------------------
        bool ExportToFile(const std::string& filePath, Asset* asset) override
        {
            std::string texPath = "";

            SpriteAtlasAsset* atlasAsset = dynamic_cast<SpriteAtlasAsset*>(asset);
            SpriteAtlas* atlas = nullptr;
            if (atlasAsset)
            {
                atlas = atlasAsset->GetInstance().get();

                std::ifstream in(filePath);

                nlohmann::json j;
                in >> j;

                texPath = j["texture"].get<std::string>();
            }
            else
            {
                atlas = new SpriteAtlas();
            }

            nlohmann::json j;

            // Texture
            const AssetRef<Texture2DAsset>& tex = atlas->GetTexture();
            j["texture"] = texPath;

            // ---------------------------------------
            // SPRITES
            // ---------------------------------------
            nlohmann::json spritesJson = nlohmann::json::object();
            const std::vector<FrameEntry>& frames = atlas->GetFrameEntries();

            for (size_t i = 0; i < frames.size(); i++)
            {
                int id = frames[i].stableId;
                const SpriteFrame& frame = frames[i].frame;

                nlohmann::json frameJson;
                frameJson["x"] = frame.UV.x;
                frameJson["y"] = frame.UV.y;
                frameJson["w"] = frame.Size.x;
                frameJson["h"] = frame.Size.y;
                frameJson["time"] = frame.FrameTime;

                spritesJson[std::to_string(id)] = frameJson;
            }

            j["sprites"] = spritesJson;

            // ---------------------------------------
            // CLIPS
            // ---------------------------------------
            nlohmann::json clipsJson = nlohmann::json::array();
            const std::vector<SpriteAnimClip>& clips = atlas->GetClips();

            for (size_t i = 0; i < clips.size(); i++)
            {
                const SpriteAnimClip& clip = clips[i];

                nlohmann::json clipJson;
                clipJson["speed"] = clip.Speed;
                clipJson["frames"] = clip.Frames;

                clipsJson.push_back(clipJson);
            }

            j["clips"] = clipsJson;

            if (!atlasAsset)
                delete atlas;

            // ---------------------------------------
            // WRITE FILE
            // ---------------------------------------
            std::ofstream file(filePath);
            if (!file)
                return false;

            file << std::setw(4) << j;

            return true;
        }

        // --------------------------------------------------------------------
        // EXTENSIONS
        // --------------------------------------------------------------------
        virtual std::vector<std::string> GetExtensions() const override
        {
            return { ".bsa" };
        }
    };
}
