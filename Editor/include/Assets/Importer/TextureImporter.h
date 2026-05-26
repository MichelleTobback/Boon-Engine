#pragma once

#include "Assets/Importer/AssetImporter.h"
#include "Asset/Runtime/BAssetFile.h"
#include "Asset/TextureAsset.h"

#include <stb_image.h>

namespace Boon
{
    class Texture2DImporter : public AssetImporter
    {
    public:
        using AssetType = Texture2DAsset;

        virtual ~Texture2DImporter() = default;

        virtual bool ImportToBAsset(
            AssetLibrary& assetLib,
            const std::filesystem::path& sourcePath, 
            const std::filesystem::path& exportPath, 
            const AssetMeta& meta) override
        {
            TextureDescriptor desc{};

            int width = 0;
            int height = 0;
            int channels = 0;

            stbi_set_flip_vertically_on_load(1);
            stbi_uc* data = stbi_load(sourcePath.string().c_str(), &width, &height, &channels, 0);
            if (!data)
                return false;

            if (channels != 3 && channels != 4)
            {
                stbi_image_free(data);
                return false;
            }

            desc.Width = static_cast<uint32_t>(width);
            desc.Height = static_cast<uint32_t>(height);
            desc.Format = channels == 4 ? ImageFormat::RGBA8 : ImageFormat::RGB8;

            Texture2DAsset asset(meta.uuid);
            asset.m_Desc = desc;
            asset.m_Data = Buffer(data, static_cast<size_t>(desc.Width) * static_cast<size_t>(desc.Height) * static_cast<size_t>(channels));

            stbi_image_free(data);

            Buffer payload = AssetSerializer<Texture2DAsset>::Serialize(&asset);
            return BAssetFile::Write(exportPath, meta, payload);
        }

        std::vector<std::string> GetExtensions() const override
        {
            return { ".png", ".jpg", ".jpeg" };
        }
    };
}
