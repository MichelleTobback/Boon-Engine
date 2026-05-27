#pragma once

#include "Assets/Importer/AssetImporter.h"

#include "Asset/AssetLibrary.h"
#include "Asset/Runtime/BAssetFile.h"
#include "Asset/MaterialAsset.h"

#include "Core/Memory/Buffer.h"

#include <filesystem>
#include <fstream>
#include <cstring>
#include <nlohmann/json.hpp>

namespace Boon
{
	class MaterialImporter : public AssetImporter
	{
	public:
		using AssetType = MaterialAsset;

		bool ImportToBAsset(
			AssetLibrary& assetLib,
			const std::filesystem::path& sourcePath,
			const std::filesystem::path& exportPath,
			const AssetMeta& meta) override
		{
			std::ifstream file(sourcePath);
			if (!file)
				return false;

			nlohmann::json j;
			file >> j;

			MaterialAsset asset(meta.uuid);

			asset.SetShader(AssetHandle(j.value("shaderHandle", uint64_t{ 0 })));

			asset.SetPrimitiveType(StringToPrimitive(j.value("primitive", "Triangles")));
			asset.SetBlendMode(StringToBlend(j.value("blend", "Alpha")));
			asset.SetDepthMode(StringToDepth(j.value("depth", "ReadWrite")));
			asset.SetCullMode(StringToCull(j.value("cull", "None")));

			if (j.contains("data") && j["data"].is_array())
			{
				Buffer data;

				for (const auto& value : j["data"])
					data.Write<float>(value.get<float>());

				asset.SetData(data);
			}

			if (j.contains("textures") && j["textures"].is_array())
			{
				for (const auto& tex : j["textures"])
				{
					const std::string name = tex.value("name", std::string{});
					const AssetHandle textureHandle = AssetHandle(tex.value("textureHandle", uint64_t{ 0 }));
					const uint32_t slot = tex.value("slot", 0u);

					if (!name.empty())
						asset.AddTexture(name, textureHandle, slot);
				}
			}

			Buffer payload = AssetSerializer<MaterialAsset>::Serialize(&asset);
			return BAssetFile::Write(exportPath, meta, payload);
		}

		bool ExportToFile(const std::filesystem::path& filePath, Asset* asset) override
		{
			nlohmann::json j;

			auto* material = dynamic_cast<MaterialAsset*>(asset);
			if (!material)
				return false;

			j["shaderHandle"] = static_cast<uint64_t>(material->GetShader());
			j["primitive"] = PrimitiveToString(material->GetPrimitiveType());
			j["blend"] = BlendToString(material->GetBlendMode());
			j["depth"] = DepthToString(material->GetDepthMode());
			j["cull"] = CullToString(material->GetCullMode());

			j["data"] = nlohmann::json::array();

			const Buffer& data = material->GetData();
			for (size_t offset = 0; offset + sizeof(float) <= data.Size(); offset += sizeof(float))
			{
				float value = 0.0f;
				std::memcpy(&value, data.DataAt(offset), sizeof(float));
				j["data"].push_back(value);
			}

			j["textures"] = nlohmann::json::array();

			for (const auto& binding : material->GetTextures())
			{
				j["textures"].push_back({
					{ "name", binding.Name },
					{ "textureHandle", static_cast<uint64_t>(binding.TextureHandle) },
					{ "slot", binding.Slot }
					});
			}

			std::filesystem::create_directories(filePath.parent_path());

			std::ofstream file(filePath);
			if (!file)
				return false;

			file << j.dump(4);
			return true;
		}

		std::vector<std::string> GetExtensions() const override
		{
			return { ".bmat" };
		}

	private:
		static PrimitiveType StringToPrimitive(const std::string& value)
		{
			if (value == "Lines")
				return PrimitiveType::Lines;

			return PrimitiveType::Triangles;
		}

		static BlendMode StringToBlend(const std::string& value)
		{
			if (value == "None")
				return BlendMode::None;

			if (value == "Additive")
				return BlendMode::Additive;

			return BlendMode::Alpha;
		}

		static DepthMode StringToDepth(const std::string& value)
		{
			if (value == "Disabled")
				return DepthMode::Disabled;

			if (value == "Read")
				return DepthMode::Read;

			return DepthMode::ReadWrite;
		}

		static CullMode StringToCull(const std::string& value)
		{
			if (value == "Back")
				return CullMode::Back;

			if (value == "Front")
				return CullMode::Front;

			return CullMode::None;
		}

		static const char* PrimitiveToString(PrimitiveType value)
		{
			switch (value)
			{
			case PrimitiveType::Lines: return "Lines";
			case PrimitiveType::Triangles:
			default: return "Triangles";
			}
		}

		static const char* BlendToString(BlendMode value)
		{
			switch (value)
			{
			case BlendMode::None: return "None";
			case BlendMode::Additive: return "Additive";
			case BlendMode::Alpha:
			default: return "Alpha";
			}
		}

		static const char* DepthToString(DepthMode value)
		{
			switch (value)
			{
			case DepthMode::Disabled: return "Disabled";
			case DepthMode::Read: return "Read";
			case DepthMode::ReadWrite:
			default: return "ReadWrite";
			}
		}

		static const char* CullToString(CullMode value)
		{
			switch (value)
			{
			case CullMode::Back: return "Back";
			case CullMode::Front: return "Front";
			case CullMode::None:
			default: return "None";
			}
		}
	};
}