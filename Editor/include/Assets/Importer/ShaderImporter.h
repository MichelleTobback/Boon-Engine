#pragma once

#include "Assets/Importer/AssetImporter.h"
#include "Asset/Runtime/BAssetFile.h"
#include "Asset/ShaderAsset.h"

#include <fstream>
#include <sstream>

namespace Boon
{
    class ShaderImporter : public AssetImporter
    {
    public:
        using AssetType = ShaderAsset;

        bool ImportToBAsset(
            const std::filesystem::path& sourcePath,
            const std::filesystem::path& exportPath,
            const AssetMeta& meta) override
        {
            std::ifstream in(sourcePath, std::ios::in);
            if (!in)
                return false;

            std::stringstream ss;
            ss << in.rdbuf();
            std::string content = ss.str();

            size_t vertPos = content.find("#vert");
            size_t fragPos = content.find("#frag");

            if (vertPos == std::string::npos || fragPos == std::string::npos)
                return false;

            ShaderAsset asset(meta.uuid);
            asset.m_VertexSource = content.substr(vertPos + 5, fragPos - (vertPos + 5));
            asset.m_FragmentSource = content.substr(fragPos + 5);

            Buffer payload = AssetSerializer<ShaderAsset>::Serialize(&asset);
            return BAssetFile::Write(exportPath, meta, payload);
        }

        std::vector<std::string> GetExtensions() const override
        {
            return { ".vert", ".frag", ".glsl", ".hlsl" };
        }
    };
}
