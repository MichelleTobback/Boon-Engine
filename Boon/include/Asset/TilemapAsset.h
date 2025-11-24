#pragma once
#include "Asset/Asset.h"
#include "Core/Memory/Buffer.h"
#include "Asset/AssetMeta.h"
#include "Renderer/Tilemap.h"

#include <memory>

namespace Boon
{
    class TilemapAsset : public Asset
    {
    public:
        using Type = Tilemap;
        TilemapAsset(AssetHandle handle, const std::shared_ptr<Tilemap>& pTilemap)
            : Asset(handle), m_pTilemap(pTilemap) {
        }
        virtual ~TilemapAsset() = default;

        inline std::shared_ptr<Tilemap> GetInstance() const { return m_pTilemap; }

    private:
        friend class TilemapImporter;
        std::shared_ptr<Tilemap> m_pTilemap{ nullptr };
    };

    template<>
    struct AssetTraits<TilemapAsset>
    {
        static constexpr AssetType Type = AssetType::Tilemap;

        // ------------------------------------------------------
        //  LOAD from AssetPack (runtime)
        // ------------------------------------------------------
        static TilemapAsset* Load(Buffer& buffer, const AssetMeta& meta)
        {
            //size_t cursor = 0;
            //
            //// Read texture UUID
            //uint32_t textureUUID = buffer.Read<uint32_t>(cursor);
            //
            //// Create atlas
            //std::shared_ptr<SpriteAtlas> atlas = std::make_shared<SpriteAtlas>();
            //
            //// Lazy texture reference
            //atlas->SetTexture(AssetRef<Texture2DAsset>(textureUUID));
            //
            //// -------------------------
            //// Load animation clips
            //// -------------------------
            //uint32_t clipCount = buffer.Read<uint32_t>(cursor);
            //
            //for (uint32_t i = 0; i < clipCount; i++)
            //{
            //    SpriteAnimClip clip;
            //
            //    clip.Speed = buffer.Read<float>(cursor);
            //
            //    uint32_t frameCount = buffer.Read<uint32_t>(cursor);
            //    clip.Frames.resize(frameCount);
            //
            //    for (uint32_t f = 0; f < frameCount; f++)
            //    {
            //        SpriteFrame frame;
            //
            //        int id = buffer.Read<uint32_t>(cursor);
            //        frame.UV.x = buffer.Read<float>(cursor);
            //        frame.UV.y = buffer.Read<float>(cursor);
            //        frame.Size.x = buffer.Read<float>(cursor);
            //        frame.Size.y = buffer.Read<float>(cursor);
            //        frame.FrameTime = buffer.Read<float>(cursor);
            //
            //        // Store full frame in clip
            //        clip.Frames[f] = id;
            //        atlas->SetSpriteFrame(frame, id);
            //    }
            //
            //    clip.pAtlas = atlas.get();
            //    atlas->AddClip(clip);
            //}

            return new TilemapAsset(meta.uuid, nullptr);
        }

        // ------------------------------------------------------
        //  SERIALIZE into AssetPack (editor build)
        // ------------------------------------------------------
        static Buffer Serialize(TilemapAsset* asset)
        {
            Buffer out;

            //SpriteAtlas* atlas = asset->GetInstance().get();
            //
            //// 1. Texture
            //out.Write<uint32_t>(atlas->GetTexture().Handle());
            //
            //// 2. Clips
            //const auto& clips = atlas->GetClips();
            //out.Write<uint32_t>((uint32_t)clips.size());
            //
            //for (const auto& clip : clips)
            //{
            //    out.Write<float>(clip.Speed);
            //    out.Write<uint32_t>((uint32_t)clip.Frames.size());
            //
            //    for (uint32_t id : clip.Frames)
            //    {
            //        const SpriteFrame& frame = atlas->GetSpriteFrame(id);
            //
            //        out.Write<uint32_t>(id);
            //        out.Write<float>(frame.UV.x);
            //        out.Write<float>(frame.UV.y);
            //        out.Write<float>(frame.Size.x);
            //        out.Write<float>(frame.Size.y);
            //        out.Write<float>(frame.FrameTime);
            //    }
            //}

            return out;
        }
    };

}
