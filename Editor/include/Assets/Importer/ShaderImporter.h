#pragma once

#include "Assets/Importer/AssetImporter.h"
#include "Asset/Runtime/BAssetFile.h"
#include "Asset/ShaderAsset.h"
#include "Renderer/ShaderCompiler/ShaderPreprocessor.h"

#include <fstream>
#include <sstream>

namespace Boon
{
    class ShaderImporter : public AssetImporter
    {
    public:
        using AssetType = ShaderAsset;

        virtual ~ShaderImporter() = default;

        bool ImportToBAsset(
            AssetLibrary& assetLib,
            const std::filesystem::path& sourcePath,
            const std::filesystem::path& exportPath,
            const AssetMeta& meta) override
        {
            std::ifstream in(sourcePath, std::ios::in);
            if (!in)
                return false;

            std::stringstream ss;
            ss << in.rdbuf();

            const std::string content = ss.str();

            const size_t vertPos = content.find("#vert");
            const size_t fragPos = content.find("#frag");

            if (vertPos == std::string::npos || fragPos == std::string::npos)
                return false;

            const std::string rawVertexSource = content.substr(vertPos + 5, fragPos - (vertPos + 5));

            const std::string rawFragmentSource = content.substr(fragPos + 5);

            ShaderPreprocessor preprocessor;

            ShaderAsset asset(meta.uuid);
            asset.m_VertexSource = preprocessor.ResolveIncludes(rawVertexSource, sourcePath.parent_path());

            asset.m_FragmentSource = preprocessor.ResolveIncludes(rawFragmentSource, sourcePath.parent_path());

            GLSLReflectionProvider reflectionProvider;
            asset.m_Reflection = reflectionProvider.Reflect(asset.m_VertexSource, asset.m_FragmentSource);

            Buffer payload = AssetSerializer<ShaderAsset>::Serialize(&asset);
            return BAssetFile::Write(exportPath, meta, payload);
        }

        std::vector<std::string> GetExtensions() const override
        {
            return { ".vert", ".frag", ".glsl", ".hlsl" };
        }
    };
}
