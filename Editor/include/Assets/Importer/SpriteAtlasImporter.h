#pragma once

#include "Assets/Importer/AssetImporter.h"
#include "Asset/AssetLibrary.h"
#include "Asset/Runtime/BAssetFile.h"
#include "Asset/SpriteAtlasAsset.h"
#include "Asset/TextureAsset.h"
#include "Core/ServiceLocator.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <unordered_map>

namespace Boon
{
    class SpriteAtlasImporter : public AssetImporter
    {
    public:
        using AssetType = SpriteAtlasAsset;

        bool ImportToBAsset(
            const std::filesystem::path& sourcePath,
            const std::filesystem::path& exportPath,
            const AssetMeta& meta) override
        {
            std::shared_ptr<SpriteAtlas> instance = std::make_shared<SpriteAtlas>();

            std::ifstream file(sourcePath);
            if (!file)
                return false;

            nlohmann::json j;
            file >> j;

            AssetLibrary& assets = ServiceLocator::Get<AssetLibrary>();

            const std::string texPath = j.value("texture", std::string{});
            if (!texPath.empty())
            {
                AssetRef<Texture2DAsset> tex = assets.Load<Texture2DAsset>(texPath);
                instance->SetTexture(tex);
            }

            std::unordered_map<int, int> oldToNew;

            if (j.contains("sprites"))
            {
                const nlohmann::json& spritesJson = j["sprites"];

                for (auto it = spritesJson.begin(); it != spritesJson.end(); ++it)
                {
                    const std::string key = it.key();
                    const nlohmann::json& entry = it.value();

                    SpriteFrame frame{};
                    frame.UV.x = entry["x"];
                    frame.UV.y = entry["y"];
                    frame.Size.x = entry["w"];
                    frame.Size.y = entry["h"];

                    const int newId = instance->AddSpriteFrame(frame);
                    const int oldId = entry.contains("id") ? entry["id"].get<int>() : std::stoi(key);
                    oldToNew[oldId] = newId;
                }
            }

            if (j.contains("clips"))
            {
                const nlohmann::json& clipsJson = j["clips"];

                for (const nlohmann::json& entry : clipsJson)
                {
                    SpriteAnimClip clip{};

                    clip.Name = entry.value("name", "Clip");
                    clip.FPS = entry.value("fps", 12.f);
                    clip.Speed = entry.value("speed", 1.f);
                    clip.pAtlas = instance.get();

                    std::vector<int> frames =
                        entry.value("frames", std::vector<int>{});

                    clip.Frames.reserve(frames.size());

                    for (int oldId : frames)
                    {
                        auto it = oldToNew.find(oldId);
                        clip.Frames.push_back(it == oldToNew.end() ? oldId : it->second);
                    }

                    instance->AddClip(clip);
                }
            }

            SpriteAtlasAsset asset(meta.uuid, instance);
            Buffer payload = AssetSerializer<SpriteAtlasAsset>::Serialize(&asset);
            return BAssetFile::Write(exportPath, meta, payload);
        }

        bool ExportToFile(const std::filesystem::path& filePath, Asset* asset) override
        {
            std::string texPath;

            SpriteAtlas* atlas = nullptr;
            std::unique_ptr<SpriteAtlas> fallbackAtlas;

            if (auto atlasAsset = dynamic_cast<SpriteAtlasAsset*>(asset))
            {
                atlas = atlasAsset->GetInstance().get();

                std::ifstream in(filePath);
                if (in)
                {
                    nlohmann::json existing;
                    in >> existing;
                    texPath = existing.value("texture", std::string{});
                }
            }
            else
            {
                fallbackAtlas = std::make_unique<SpriteAtlas>();
                atlas = fallbackAtlas.get();
            }

            nlohmann::json j;
            j["texture"] = texPath;

            nlohmann::json spritesJson = nlohmann::json::object();
            const std::vector<FrameEntry>& frames = atlas->GetFrameEntries();

            for (const FrameEntry& entry : frames)
            {
                nlohmann::json frameJson;
                frameJson["x"] = entry.frame.UV.x;
                frameJson["y"] = entry.frame.UV.y;
                frameJson["w"] = entry.frame.Size.x;
                frameJson["h"] = entry.frame.Size.y;

                spritesJson[std::to_string(entry.stableId)] = frameJson;
            }

            j["sprites"] = spritesJson;

            nlohmann::json clipsJson = nlohmann::json::array();

            for (const SpriteAnimClip& clip : atlas->GetClips())
            {
                nlohmann::json clipJson;

                clipJson["name"] = clip.Name;
                clipJson["fps"] = clip.FPS;
                clipJson["speed"] = clip.Speed;
                clipJson["frames"] = clip.Frames;

                clipsJson.push_back(clipJson);
            }

            j["clips"] = clipsJson;

            std::filesystem::create_directories(filePath.parent_path());
            std::ofstream file(filePath);
            if (!file)
                return false;

            file << j.dump(4);
            return true;
        }

        std::vector<std::string> GetExtensions() const override
        {
            return { ".bsa" };
        }
    };
}
