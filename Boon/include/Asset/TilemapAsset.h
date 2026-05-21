#pragma once

#include "Asset/Asset.h"
#include "Asset/AssetMeta.h"
#include "Asset/AssetSerializer.h"
#include "Asset/AssetTraits.h"
#include "Core/Memory/Buffer.h"
#include "Renderer/Tilemap.h"

#include <memory>

namespace Boon
{
    class TilemapAsset : public Asset
    {
    public:
        using Type = Tilemap;

        TilemapAsset(AssetHandle handle, const std::shared_ptr<Tilemap>& tilemap)
            : Asset(handle), m_pTilemap(tilemap)
        {
        }

        std::shared_ptr<Tilemap> GetInstance() const
        {
            return m_pTilemap;
        }

    private:
        std::shared_ptr<Tilemap> m_pTilemap = nullptr;

        friend class TilemapImporter;
        friend struct AssetSerializer<TilemapAsset>;
    };

    template<>
    struct AssetTraits<TilemapAsset>
    {
        static constexpr AssetType Type = AssetType::Tilemap;
        static constexpr const char* Name = "Tilemap";
    };

    template<>
    struct AssetSerializer<TilemapAsset>
    {
        static TilemapAsset* Load(Buffer& buffer, const AssetMeta& meta)
        {
            size_t cursor = 0;

            const int chunksX = buffer.Read<int>(cursor);
            const int chunksY = buffer.Read<int>(cursor);
            const int chunkSize = buffer.Read<int>(cursor);

            const AssetHandle atlasHandle = buffer.Read<AssetHandle>(cursor);

            std::shared_ptr<Tilemap> tilemap = std::make_shared<Tilemap>(chunksX, chunksY, chunkSize);

            tilemap->SetAtlas(AssetRef<SpriteAtlasAsset>(atlasHandle));

            const int width = chunksX * chunkSize;
            const int height = chunksY * chunkSize;

            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    const int sprite = buffer.Read<int>(cursor);
                    tilemap->SetTile(x, y, sprite);
                }
            }

            return new TilemapAsset(meta.uuid, tilemap);
        }

        static Buffer Serialize(TilemapAsset* asset)
        {
            Buffer out;

            if (!asset)
                return out;

            std::shared_ptr<Tilemap> tilemap = asset->GetInstance();
            if (!tilemap)
                return out;

            const int chunksX = tilemap->GetChunksX();
            const int chunksY = tilemap->GetChunksY();
            const int chunkSize = tilemap->GetChunkSize();

            out.Write<int>(chunksX);
            out.Write<int>(chunksY);
            out.Write<int>(chunkSize);

            out.Write<AssetHandle>(tilemap->GetAtlas().Handle());

            const int width = chunksX * chunkSize;
            const int height = chunksY * chunkSize;

            for (int y = 0; y < height; ++y)
            {
                for (int x = 0; x < width; ++x)
                {
                    out.Write<int>(tilemap->GetTile(x, y));
                }
            }

            return out;
        }
    };
}
