#pragma once
#include "Asset.h"
#include "AssetLoader.h"
#include "Renderer/SpriteAtlas.h"

namespace Boon
{
	class SpriteAtlasAsset final : public Asset
	{
	public:
		using Type = SpriteAtlas;

		SpriteAtlasAsset(const SpriteAtlasAsset& other) = delete;
		SpriteAtlasAsset(SpriteAtlasAsset&& other) = default;
		SpriteAtlasAsset& operator=(const SpriteAtlasAsset& other) = delete;
		SpriteAtlasAsset& operator=(SpriteAtlasAsset&& other) = delete;

		virtual ~SpriteAtlasAsset() = default;

		std::shared_ptr<SpriteAtlas> GetInstance() const;

	protected:
		static std::unique_ptr<SpriteAtlasAsset> Create(AssetHandle handle, const std::shared_ptr<SpriteAtlas>& pAtlas);

	private:
		friend class SpriteAtlasAssetLoader;
		SpriteAtlasAsset(AssetHandle handle, const std::shared_ptr<SpriteAtlas>& pAtlas);

		std::shared_ptr<SpriteAtlas> m_pAtlas{ nullptr };
	};

	class SpriteAtlasAssetLoader final : public AssetLoader
	{
	public:
		virtual ~SpriteAtlasAssetLoader() = default;

		SpriteAtlasAssetLoader(const SpriteAtlasAssetLoader& other) = default;
		SpriteAtlasAssetLoader(SpriteAtlasAssetLoader&& other) = default;
		SpriteAtlasAssetLoader& operator=(const SpriteAtlasAssetLoader& other) = delete;
		SpriteAtlasAssetLoader& operator=(SpriteAtlasAssetLoader&& other) = delete;

		virtual std::unique_ptr<Asset> Load(const std::string& path) override;

	protected:
		static std::unique_ptr<SpriteAtlasAssetLoader> Create();

	private:
		friend class AssetLibrary;
		SpriteAtlasAssetLoader()
			: AssetLoader({ "bsa" }) {}
	};
}