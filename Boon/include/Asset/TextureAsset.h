#pragma once
#include "Asset/Asset.h"
#include "Asset/AssetTraits.h"
#include "Asset/AssetMeta.h"
#include "Core/Memory/Buffer.h"
#include "Renderer/Texture.h"

#include <memory>

namespace Boon
{
    class Texture2DAsset : public Asset
    {
    public:
        using Type = Texture2D;
        Texture2DAsset(AssetHandle handle)
            : Asset(handle) {
        }

        std::shared_ptr<Texture2D> GetInstance()
        {
            if (!m_RuntimeTexture)
                CreateRuntimeTexture();
            return m_RuntimeTexture;
        }

        uint32_t GetWidth() const { return m_Desc.Width; }
        uint32_t GetHeight() const { return m_Desc.Height; }
        const Buffer& GetPixelData() const { return m_Data; }
        const TextureDescriptor& GetDescriptor() const { return m_Desc; }

    private:
        void CreateRuntimeTexture()
        {
            m_RuntimeTexture = Texture2D::Create(m_Desc);
            m_RuntimeTexture->SetData(m_Data);
        }

    private:
        TextureDescriptor m_Desc;
        Buffer m_Data;

        std::shared_ptr<Texture2D> m_RuntimeTexture = nullptr;

        friend class Texture2DImporter;
        friend class AssetTraits<Texture2DAsset>;
    };

    template<>
    struct AssetTraits<Texture2DAsset>
    {
        static constexpr AssetType Type = AssetType::Texture;

        static Texture2DAsset* Load(Buffer& buffer, const AssetMeta& meta)
        {
            Texture2DAsset* asset = new Texture2DAsset(meta.uuid);

            // Cursor tracking
            size_t cursor = 0;

            // Read descriptor
            asset->m_Desc = buffer.Read<TextureDescriptor>(cursor);

            // Read number of channels (stored as uint32_t)
            uint32_t channels = buffer.Read<uint32_t>(cursor);

            // Determine correct format
            if (channels == 4)
                asset->m_Desc.Format = ImageFormat::RGBA8;
            else if (channels == 3)
                asset->m_Desc.Format = ImageFormat::RGB8;
            else
            {
                delete asset;
                return nullptr; // invalid texture format
            }

            // Compute pixel data size
            size_t pixelCount = asset->m_Desc.Width * asset->m_Desc.Height * channels;

            // Allocate buffer appropriately
            asset->m_Data.Reserve(pixelCount);

            // Read pixel bytes
            buffer.ReadRaw(asset->m_Data.Data(), pixelCount, cursor);

            return asset;
        }

        static Buffer Serialize(Texture2DAsset* asset)
        {
            Buffer out;

            // Write descriptor
            out.Write(asset->m_Desc);

            // Write channels based on format
            uint32_t channels = 4;
            if (asset->m_Desc.Format == ImageFormat::RGB8)
                channels = 3;

            out.Write(channels);

            // Write pixel data
            out.WriteRaw(asset->GetPixelData().Data(),
                asset->GetPixelData().Size());

            return out;
        }
    };

}
