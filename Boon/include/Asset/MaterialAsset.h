#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetMeta.h"
#include "Asset/AssetSerializer.h"
#include "Asset/AssetTraits.h"
#include "Asset/AssetRef.h"

#include "Asset/ShaderAsset.h"
#include "Asset/TextureAsset.h"

#include "Core/Memory/Buffer.h"

#include "Renderer/Material.h"
#include "Renderer/MaterialFactory.h"
#include "Renderer/Pipeline.h"

#include <memory>
#include <string>
#include <vector>

namespace Boon
{
	class MaterialAsset : public Asset
	{
	public:
		struct TextureBinding
		{
			std::string Name;
			AssetHandle TextureHandle = 0;
			uint32_t Slot = 0;
		};

	public:
		MaterialAsset(AssetHandle handle)
			: Asset(handle)
		{
		}

		void SetShader(AssetHandle shaderHandle)
		{
			m_ShaderHandle = shaderHandle;

			Touch();
		}

		AssetHandle GetShader() const
		{
			return m_ShaderHandle;
		}

		void SetData(const Buffer& data)
		{
			m_Data = data;

			Touch();
		}

		const Buffer& GetData() const
		{
			return m_Data;
		}

		void AddTexture(const std::string& name, AssetHandle textureHandle, uint32_t slot)
		{
			m_Textures.push_back({ name, textureHandle, slot });

			Touch();
		}

		void SetBlendMode(BlendMode mode)
		{
			m_Blend = mode;
			Touch();
		}

		void SetDepthMode(DepthMode mode)
		{
			m_Depth = mode;
			Touch();
		}

		void SetCullMode(CullMode mode)
		{
			m_Cull = mode;
			Touch();
		}

		void SetPrimitiveType(PrimitiveType type)
		{
			m_Primitive = type;
			Touch();
		}

		PrimitiveType GetPrimitiveType() const { return m_Primitive; }
		BlendMode GetBlendMode() const { return m_Blend; }
		DepthMode GetDepthMode() const { return m_Depth; }
		CullMode GetCullMode() const { return m_Cull; }

		const std::vector<TextureBinding>& GetTextures() const { return m_Textures; }

		std::shared_ptr<Material> GetInstance()
		{
			if (!m_RuntimeMaterial || m_RuntimeMaterialVersion != m_Version)
			{
				m_RuntimeMaterial = CreateMaterial();
				m_RuntimeMaterialVersion = m_Version;
			}

			return m_RuntimeMaterial;
		}

		uint32_t GetVersion() const { return m_Version; }

		std::shared_ptr<Material> CreateMaterial() const
		{
			if (!m_ShaderHandle.IsValid())
				return nullptr;

			AssetRef<ShaderAsset> shaderRef{ m_ShaderHandle };
			if (!shaderRef.IsValid())
				return nullptr;

			ShaderAsset* shaderAsset = shaderRef.Get();
			if (!shaderAsset)
				return nullptr;

			auto material = MaterialFactory::CreateFromShaderAsset(
				*shaderAsset,
				m_Blend,
				m_Depth,
				m_Cull,
				m_Primitive);

			if (!material)
				return nullptr;

			if (!m_Data.Empty())
				material->SetRaw(0, m_Data.Data(), m_Data.Size());

			for (const TextureBinding& binding : m_Textures)
			{
				if (!binding.TextureHandle.IsValid())
					continue;

				AssetRef<Texture2DAsset> textureRef{ binding.TextureHandle };
				if (textureRef.IsValid())
					material->SetTexture(binding.Name, textureRef.Instance(), binding.Slot);
			}

			return material;
		}

		void SetTextureBinding(size_t index, const std::string& name, AssetHandle textureHandle, uint32_t slot)
		{
			if (index >= m_Textures.size())
				return;

			m_Textures[index].Name = name;
			m_Textures[index].TextureHandle = textureHandle;
			m_Textures[index].Slot = slot;
			m_RuntimeMaterial = nullptr;
			Touch();
		}

	private:
		friend class MaterialImporter;
		friend struct AssetSerializer<MaterialAsset>;

		void Touch()
		{
			++m_Version;
		}

		AssetHandle m_ShaderHandle = 0;
		uint32_t m_Version = 1;

		PrimitiveType m_Primitive = PrimitiveType::Triangles;
		BlendMode m_Blend = BlendMode::Alpha;
		DepthMode m_Depth = DepthMode::ReadWrite;
		CullMode m_Cull = CullMode::None;

		Buffer m_Data;
		std::vector<TextureBinding> m_Textures;

		uint32_t m_RuntimeMaterialVersion = 0;
		std::shared_ptr<Material> m_RuntimeMaterial = nullptr;
	};

	template<>
	struct AssetTraits<MaterialAsset>
	{
		static constexpr AssetType Type = AssetType::Material;
		static constexpr const char* Name = "Material";
	};

	template<>
	struct AssetSerializer<MaterialAsset>
	{
		static MaterialAsset* Load(Buffer& buffer, const AssetMeta& meta)
		{
			size_t cursor = 0;

			auto* asset = new MaterialAsset(meta.uuid);

			asset->m_ShaderHandle = buffer.Read<AssetHandle>(cursor);

			asset->m_Primitive = static_cast<PrimitiveType>(buffer.Read<int>(cursor));
			asset->m_Blend = static_cast<BlendMode>(buffer.Read<int>(cursor));
			asset->m_Depth = static_cast<DepthMode>(buffer.Read<int>(cursor));
			asset->m_Cull = static_cast<CullMode>(buffer.Read<int>(cursor));

			const uint32_t dataSize = buffer.Read<uint32_t>(cursor);
			if (dataSize > 0)
			{
				asset->m_Data.Resize(dataSize);
				buffer.ReadRaw(asset->m_Data.Data(), dataSize, cursor);
			}

			const uint32_t textureCount = buffer.Read<uint32_t>(cursor);
			for (uint32_t i = 0; i < textureCount; ++i)
			{
				MaterialAsset::TextureBinding binding{};
				binding.Name = buffer.ReadString(cursor);
				binding.TextureHandle = buffer.Read<AssetHandle>(cursor);
				binding.Slot = buffer.Read<uint32_t>(cursor);

				asset->m_Textures.push_back(binding);
			}

			return asset;
		}

		static Buffer Serialize(MaterialAsset* asset)
		{
			Buffer out;

			if (!asset)
				return out;

			out.Write<AssetHandle>(asset->m_ShaderHandle);

			out.Write<int>(static_cast<int>(asset->m_Primitive));
			out.Write<int>(static_cast<int>(asset->m_Blend));
			out.Write<int>(static_cast<int>(asset->m_Depth));
			out.Write<int>(static_cast<int>(asset->m_Cull));

			out.Write<uint32_t>(static_cast<uint32_t>(asset->m_Data.Size()));
			out.WriteRaw(asset->m_Data.Data(), asset->m_Data.Size());

			out.Write<uint32_t>(static_cast<uint32_t>(asset->m_Textures.size()));

			for (const MaterialAsset::TextureBinding& binding : asset->m_Textures)
			{
				out.WriteString(binding.Name);
				out.Write<AssetHandle>(binding.TextureHandle);
				out.Write<uint32_t>(binding.Slot);
			}

			return out;
		}
	};
}