#pragma once
#include "AssetImporter.h"
#include "Asset/TextureAsset.h"
#include <stb_image.h>

namespace Boon
{
    class Texture2DImporter : public AssetImporter
    {
    public:
        using AssetType = Texture2DAsset;

        Asset* ImportFromFile(const std::string& filePath, const AssetMeta& meta) override
        {
            Texture2DAsset* pResult{ nullptr };
            TextureDescriptor desc{};

            int width, height, channels;
            stbi_set_flip_vertically_on_load(1);
            stbi_uc* data = nullptr;
            {
                data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
            }

            if (data)
            {
                desc.Width = width;
                desc.Height = height;

                pResult = new Texture2DAsset(meta.uuid);

                if (channels == 4)
                {
                    desc.Format = ImageFormat::RGBA8;
                }
                else if (channels == 3)
                {
                    desc.Format = ImageFormat::RGB8;
                }
                pResult->m_Desc = desc;
                pResult->m_Data = Buffer(data, desc.Width * desc.Height * channels);

                stbi_image_free(data);
            }
            return pResult;
        }

        virtual std::vector<std::string> GetExtensions() const override
        {
            return { ".png", ".jpg" };
        }
    };
}
