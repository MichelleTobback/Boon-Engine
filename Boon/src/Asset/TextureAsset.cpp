#include "Asset/TextureAsset.h"

#include <filesystem>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace Boon;

Boon::Texture2DAsset::Texture2DAsset(AssetHandle handle, const std::shared_ptr<Texture2D>& pTexture)
	: Asset(handle), m_pTexture{ pTexture }{}

std::shared_ptr<Texture2D> Boon::Texture2DAsset::GetInstance() const { return m_pTexture; }

std::unique_ptr<Texture2DAsset> Boon::Texture2DAsset::Create(AssetHandle handle, const std::shared_ptr<Texture2D>& pTexture)
{
	return std::make_unique<Texture2DAsset>(Texture2DAsset(handle, pTexture));
}

std::unique_ptr<Asset> Boon::Texture2DAssetLoader::Load(const std::string& path)
{
    std::unique_ptr<Texture2DAsset> pResult{ nullptr };

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

    TextureDescriptor desc{};

    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);
    stbi_uc* data = nullptr;
    {
        data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    }

	if (data)
	{
        desc.Width = width;
		desc.Height = height;

        pResult = Texture2DAsset::Create(handle, Texture2D::Create(desc));

		if (channels == 4)
		{
            desc.Format = ImageFormat::RGBA8;
		}
		else if (channels == 3)
		{
            desc.Format = ImageFormat::RGB8;
		}

        pResult->m_pTexture->SetData(data, desc.Width * desc.Height * channels);

		stbi_image_free(data);
	}

    return std::move(pResult);
}

std::unique_ptr<Texture2DAssetLoader> Boon::Texture2DAssetLoader::Create()
{
	return std::make_unique<Texture2DAssetLoader>(Texture2DAssetLoader());
}
