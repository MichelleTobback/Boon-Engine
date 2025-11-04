#include "Asset/SpriteAtlasAsset.h"

#include "Asset/AssetLibrary.h"
#include "Asset/TextureAsset.h"
#include "Core/ServiceLocator.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace Boon;

std::shared_ptr<SpriteAtlas> Boon::SpriteAtlasAsset::GetInstance() const
{
	return m_pAtlas;
}

std::unique_ptr<SpriteAtlasAsset> Boon::SpriteAtlasAsset::Create(AssetHandle handle, const std::shared_ptr<SpriteAtlas>& pAtlas)
{
    return std::make_unique<SpriteAtlasAsset>(SpriteAtlasAsset(handle, pAtlas));
}

Boon::SpriteAtlasAsset::SpriteAtlasAsset(AssetHandle handle, const std::shared_ptr<SpriteAtlas>& pAtlas)
    : Asset(handle), m_pAtlas{ pAtlas }
{
}

std::unique_ptr<Asset> Boon::SpriteAtlasAssetLoader::Load(const std::string& path)
{
    std::unique_ptr<SpriteAtlasAsset> pResult = nullptr;
    std::shared_ptr<SpriteAtlas> pInstance = std::make_shared<SpriteAtlas>();

    std::ifstream file(path);
    nlohmann::json j;
    file >> j;

    AssetHandle handle{j["uuid"] == 0 ? AssetHandle() : AssetHandle(j["uuid"]) };

    AssetLibrary& assets = ServiceLocator::Get<AssetLibrary>();
    pInstance->SetTexture(assets.GetAsset<Texture2DAsset>(assets.Load<Texture2DAssetLoader>(j["texture"].get<std::string>())));

    for (auto& [name, entry] : j["sprites"].items())
    {
        SpriteFrame uv;
        uv.UV = { entry["x"], entry["y"] };
        uv.Size = { entry["w"], entry["h"] };
        uv.FrameTime = entry["time"];
        pInstance->SetSpriteFrame(uv, entry["id"]);
    }

    for (auto& [name, entry] : j["clips"].items())
    {
        SpriteAnimClip clip;
        clip.Frames = entry["frames"].get<std::vector<int>>();
        clip.Speed = entry["speed"];
        clip.pAtlas = pInstance.get();
        pInstance->AddClip(clip);
    }

    pResult = SpriteAtlasAsset::Create(handle, pInstance);

	return std::move(pResult);
}

std::unique_ptr<SpriteAtlasAssetLoader> Boon::SpriteAtlasAssetLoader::Create()
{
	return std::make_unique<SpriteAtlasAssetLoader>(SpriteAtlasAssetLoader());
}
