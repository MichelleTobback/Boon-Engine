#pragma once
#include "Asset.h"
#include "AssetLoader.h"
#include "Renderer/Texture.h"

namespace Boon
{
	class Texture2DAsset final : public Asset
	{
	public:
		using Type = Texture2D;

		Texture2DAsset(const Texture2DAsset& other) = delete;
		Texture2DAsset(Texture2DAsset&& other) = default;
		Texture2DAsset& operator=(const Texture2DAsset& other) = delete;
		Texture2DAsset& operator=(Texture2DAsset&& other) = delete;

		virtual ~Texture2DAsset() = default;

		std::shared_ptr<Texture2D> GetInstance() const;

	protected:
		static std::unique_ptr<Texture2DAsset> Create(AssetHandle handle, const std::shared_ptr<Texture2D>& pTexture);

	private:
		friend class Texture2DAssetLoader;
		Texture2DAsset(AssetHandle handle, const std::shared_ptr<Texture2D>& pTexture);

		std::shared_ptr<Texture2D> m_pTexture{ nullptr };
	};

	class Texture2DAssetLoader final : public AssetLoader
	{
	public:
		virtual ~Texture2DAssetLoader() = default;

		Texture2DAssetLoader(const Texture2DAssetLoader& other) = default;
		Texture2DAssetLoader(Texture2DAssetLoader&& other) = default;
		Texture2DAssetLoader& operator=(const Texture2DAssetLoader& other) = delete;
		Texture2DAssetLoader& operator=(Texture2DAssetLoader&& other) = delete;

		virtual std::unique_ptr<Asset> Load(const std::string& path) override;

	protected:
		static std::unique_ptr<Texture2DAssetLoader> Create();

	private:
		friend class AssetLibrary;
		Texture2DAssetLoader()
			: AssetLoader({ "png", "jpg", "jpeg" }) {}
	};
}