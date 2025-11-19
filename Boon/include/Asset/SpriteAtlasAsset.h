#pragma once
#include "Asset/Asset.h"
#include "Core/Memory/Buffer.h"
#include "Asset/AssetMeta.h"
#include "Renderer/SpriteAtlas.h"

#include <memory>

namespace Boon
{
    class SpriteAtlasAsset : public Asset
    {
    public:
        using Type = SpriteAtlas;
        SpriteAtlasAsset(AssetHandle handle, const std::shared_ptr<SpriteAtlas>& pAtlas)
            : Asset(handle), m_pAtlas(pAtlas) { }
        virtual ~SpriteAtlasAsset() = default;

        inline std::shared_ptr<SpriteAtlas> GetInstance() const { return m_pAtlas; }

    private:
        friend class SpriteAtlasImporter;
        std::shared_ptr<SpriteAtlas> m_pAtlas{ nullptr };
    };

    template<>
    struct AssetTraits<SpriteAtlasAsset>
    {
        static constexpr AssetType Type = AssetType::SpriteAtlas;

        // ------------------------------------------------------
        //  LOAD from AssetPack (runtime)
        // ------------------------------------------------------
        static SpriteAtlasAsset* Load(Buffer& buffer, const AssetMeta& meta)
        {
            size_t cursor = 0;

            // Read texture UUID
            uint32_t textureUUID = buffer.Read<uint32_t>(cursor);

            // Create atlas
            std::shared_ptr<SpriteAtlas> atlas = std::make_shared<SpriteAtlas>();

            // Lazy texture reference
            atlas->SetTexture(AssetRef<Texture2DAsset>(textureUUID));

            // -------------------------
            // Load animation clips
            // -------------------------
            uint32_t clipCount = buffer.Read<uint32_t>(cursor);

            for (uint32_t i = 0; i < clipCount; i++)
            {
                SpriteAnimClip clip;

                clip.Speed = buffer.Read<float>(cursor);

                uint32_t frameCount = buffer.Read<uint32_t>(cursor);
                clip.Frames.resize(frameCount);

                for (uint32_t f = 0; f < frameCount; f++)
                {
                    SpriteFrame frame;

                    int id = buffer.Read<uint32_t>(cursor);
                    frame.UV.x = buffer.Read<float>(cursor);
                    frame.UV.y = buffer.Read<float>(cursor);
                    frame.Size.x = buffer.Read<float>(cursor);
                    frame.Size.y = buffer.Read<float>(cursor);
                    frame.FrameTime = buffer.Read<float>(cursor);

                    // Store full frame in clip
                    clip.Frames[f] = id;
                    atlas->SetSpriteFrame(frame, id);
                }

                clip.pAtlas = atlas.get();
                atlas->AddClip(clip);
            }

            return new SpriteAtlasAsset(meta.uuid, atlas);
        }

        // ------------------------------------------------------
        //  SERIALIZE into AssetPack (editor build)
        // ------------------------------------------------------
        static Buffer Serialize(SpriteAtlasAsset* asset)
        {
            Buffer out;

            SpriteAtlas* atlas = asset->GetInstance().get();

            // 1. Texture
            out.Write<uint32_t>(atlas->GetTexture().Handle());

            // 2. Clips
            const auto& clips = atlas->GetClips();
            out.Write<uint32_t>((uint32_t)clips.size());

            for (const auto& clip : clips)
            {
                out.Write<float>(clip.Speed);
                out.Write<uint32_t>((uint32_t)clip.Frames.size());

                for (uint32_t id : clip.Frames)
                {
                    const SpriteFrame& frame = atlas->GetSpriteFrame(id);

                    out.Write<uint32_t>(id);
                    out.Write<float>(frame.UV.x);
                    out.Write<float>(frame.UV.y);
                    out.Write<float>(frame.Size.x);
                    out.Write<float>(frame.Size.y);
                    out.Write<float>(frame.FrameTime);
                }
            }

            return out;
        }
    };

}
