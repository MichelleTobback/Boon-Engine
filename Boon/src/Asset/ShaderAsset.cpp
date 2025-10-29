#include "Asset/ShaderAsset.h"
#include "Renderer/Shader.h"
#include <filesystem>
#include <fstream>
#include <sstream>

std::shared_ptr<Boon::Shader> Boon::ShaderAsset::GetInstance() const
{
    return m_pShader;
}

std::unique_ptr<Boon::ShaderAsset> Boon::ShaderAsset::Create(AssetHandle handle, const std::shared_ptr<Shader>& pShader)
{
    return std::make_unique<ShaderAsset>(ShaderAsset(handle, pShader));
}

Boon::ShaderAsset::ShaderAsset(AssetHandle handle, const std::shared_ptr<Shader>& pShader)
    : Asset(handle), m_pShader{ pShader }
{

}

std::unique_ptr<Boon::Asset> Boon::ShaderAssetLoader::Load(const std::string& path)
{
    std::unique_ptr<Boon::ShaderAsset> pResult{ nullptr };

    AssetHandle handle{ 0u };

    std::filesystem::path meta{ path + std::string(".meta") };
    if (!std::filesystem::exists(meta))
    {
        if (std::ofstream outputFile(meta); outputFile.is_open())
        {
            handle = AssetHandle();
            outputFile << uint64_t(handle);
            outputFile.close();
        }
    }
    else if (std::ifstream inputFile(meta); inputFile.is_open())
    {
        std::string handleStr{};
        std::getline(inputFile, handleStr);
        handle = std::stoull(handleStr);
    }

    std::string vert, frag;
    ReadShaderFile(path, vert, frag);

    pResult = ShaderAsset::Create(handle, Shader::Create(vert, frag));

    return std::move(pResult);
}

std::unique_ptr<Boon::ShaderAssetLoader> Boon::ShaderAssetLoader::Create()
{
    return std::make_unique<ShaderAssetLoader>(ShaderAssetLoader());
}

bool Boon::ShaderAssetLoader::ReadShaderFile(const std::string& filepath, std::string& vert, std::string& frag)
{
    std::ifstream in(filepath, std::ios::in);
    if (!in)
        return false;

    std::stringstream ss;
    ss << in.rdbuf();
    std::string content = ss.str();
    in.close();

    size_t vertPos = content.find("#vert");
    size_t fragPos = content.find("#frag");

    if (vertPos == std::string::npos || fragPos == std::string::npos)
        return false;

    vert = content.substr(vertPos + 5, fragPos - (vertPos + 5));
    frag = content.substr(fragPos + 5);

    return true;
}
