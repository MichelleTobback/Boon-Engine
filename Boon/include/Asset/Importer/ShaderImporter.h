#pragma once
#include "AssetImporter.h"
#include "Asset/ShaderAsset.h"
#include "Asset/AssetMeta.h"
#include <fstream>
#include <sstream>

namespace Boon
{
    class ShaderImporter : public AssetImporter
    {
    public:
        using AssetType = ShaderAsset;

        Asset* ImportFromFile(const std::string& filepath, const AssetMeta& meta) override
        {
            ShaderAsset* asset{nullptr};

            std::ifstream in(filepath, std::ios::in);

            if (!in)
                return asset;

            std::stringstream ss;
            ss << in.rdbuf();
            std::string content = ss.str();
            in.close();

            size_t vertPos = content.find("#vert");
            size_t fragPos = content.find("#frag");

            if (vertPos == std::string::npos || fragPos == std::string::npos)
                return false;

            std::string vert = content.substr(vertPos + 5, fragPos - (vertPos + 5));
            std::string frag = content.substr(fragPos + 5);

            asset = new ShaderAsset(meta.uuid);
            asset->m_VertexSource = vert;
            asset->m_FragmentSource = frag;

            return asset;
        }

        virtual std::vector<std::string> GetExtensions() const override
        {
            return { ".vert", ".frag", ".glsl", ".hlsl" };
        }
    };
}
