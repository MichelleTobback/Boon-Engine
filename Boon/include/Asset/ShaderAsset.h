#pragma once
#include "Asset.h"
#include "AssetLoader.h"
#include "Renderer/Shader.h"

namespace Boon
{
	class ShaderAsset final : public Asset
	{
	public:
		using Type = Shader;

		ShaderAsset(const ShaderAsset& other) = delete;
		ShaderAsset(ShaderAsset&& other) = default;
		ShaderAsset& operator=(const ShaderAsset& other) = delete;
		ShaderAsset& operator=(ShaderAsset&& other) = delete;

		virtual ~ShaderAsset() = default;

		std::shared_ptr<Shader> GetInstance() const;

	protected:
		static std::unique_ptr<ShaderAsset> Create(AssetHandle handle, const std::shared_ptr<Shader>& pShader);

	private:
		friend class ShaderAssetLoader;
		ShaderAsset(AssetHandle handle, const std::shared_ptr<Shader>& pShader);

		std::shared_ptr<Shader> m_pShader{ nullptr };
	};

	class ShaderAssetLoader final : public AssetLoader
	{
	public:
		virtual ~ShaderAssetLoader() = default;

		ShaderAssetLoader(const ShaderAssetLoader& other) = default;
		ShaderAssetLoader(ShaderAssetLoader&& other) = default;
		ShaderAssetLoader& operator=(const ShaderAssetLoader& other) = delete;
		ShaderAssetLoader& operator=(ShaderAssetLoader&& other) = delete;

		virtual std::unique_ptr<Asset> Load(const std::string& path) override;

	protected:
		static std::unique_ptr<ShaderAssetLoader> Create();

	private:
		friend class AssetLibrary;
		ShaderAssetLoader()
			: AssetLoader({ "vert", "frag", "glsl", "hlsl" }) {}

		bool ReadShaderFile(const std::string& filepath, std::string& vert, std::string& frag);
	};
}